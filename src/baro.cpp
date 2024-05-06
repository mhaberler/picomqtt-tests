#include "baro.hpp"
#include "params.hpp"
#include "meteo.hpp"
#include "hKalF_acc.h"
#include "TimerStats.hpp"
#include "ArduinoJson.h"
#include "broker.hpp"

QueueHandle_t irq_queue;
espidf::RingBuffer *baro_queue;

uint32_t irq_queue_full, baro_queue_full, commit_fail;


// first level interrupt handler
// only notify 2nd level handler task passing any parameters
static void IRAM_ATTR  irq_handler(void *param) {
    irqmsg_t msg;
    msg.dev = static_cast<dps_sensors_t *>(param);
    msg.timestamp = micros();
    if (xQueueSendFromISR(irq_queue, (const void*) &msg, NULL) != pdTRUE) {
        irq_queue_full++;
    }
}

// 2nd level interrupt handler
// runs in user context - can do Wire I/O, log etc
void soft_irq(void* arg) {
    irqmsg_t msg;

    for (;;) {
        if (xQueueReceive(irq_queue, &msg, portMAX_DELAY)) {
            float value;
            int16_t ret ;
            dps_sensors_t *dev = msg.dev;
            Dps3xx *dps = dev->sensor;

            if ((ret = dps->getSingleResult(value)) != 0) {
                log_e("getSingleResult: %d",ret);
                continue;
            }

            if ((ret = dps->getIntStatusPrsReady()) < 0) {
                log_e("getIntStatusPrsReady: %d",ret);
                continue;
            }

            // a pressure sample is ready
            baroSample_t *bs = nullptr;
            size_t sz = sizeof(baroSample_t);
            if (baro_queue->send_acquire((void **)&bs, sz, 0) != pdTRUE) {
                baro_queue_full++;
            } else  {
                bs->dev_id = dev->dev_id;
                bs->dev = dev;
                bs->timestamp = msg.timestamp;
                if (ret == 1) {
                    bs->type  = SAMPLE_PRESSURE;
                    bs->value = value / 100.0f;  // scale to hPa
                } else {
                    bs->type  = SAMPLE_TEMPERATURE;
                    bs->value = value; // already degC
                }
                if (baro_queue->send_complete(bs) != pdTRUE) {
                    commit_fail++;
                }
            }


            // // ret == 0 || ret == 1
            // log_i("dev=0x%x timestamp=%u %s=%.2f %s",
            //       dev->i2caddr, msg.timestamp,
            //       ret ? "pressure" : "temperature",
            //       value,
            //       ret ? "kPa"  : "°C");

            // start a new measurement cycle
            // every count & TEMP_COUNT_MASK pressure measurements start a temperature measurement
            // to keep correction accurate
            dev->softirq_count++;
            if ((dev->softirq_count & dev->temp_measure_mask)) {
                if ((ret = dps->startMeasurePressureOnce(dev->prs_osr)) != 0) {
                    log_e("startMeasurePressureOnce: %d",ret);
                }
            } else {
                if ((ret = dps->startMeasureTempOnce(dev->temp_osr)) != 0) {
                    log_e("startMeasureTempOnce: %d",ret);
                }
            }
        }
    }
}

int32_t baro_setup(void) {
    int16_t ret;
    int32_t num_devs = 0;

    irq_queue = xQueueCreate(10, sizeof(irqmsg_t));
    baro_queue = new espidf::RingBuffer();
    baro_queue->create(BARO_QUEUELEN, RINGBUF_TYPE_NOSPLIT);
    xTaskCreate(soft_irq, "soft_irq", 2048, NULL, 10, NULL);

    for (auto i = 0; i < num_sensors; i++) {
        dps_sensors_t *dev = &dps_sensors[i];
        Dps3xx *dps = dev->sensor = new Dps3xx();
        dev->dev_id = i;

        dps->begin(*dev->wire, dev->i2caddr);
        if ((ret = dps->standby()) != DPS__SUCCEEDED) {
            log_e("standby failed: %d", ret);
            delete dps;
            continue;
        }

        pinMode(dev->irq_pin, INPUT);
        uint8_t polarity;
        if (dev->i2caddr == 0x77) {
            // on standard address
            attachInterruptArg(digitalPinToInterrupt(dev->irq_pin), irq_handler, (void *)dev, RISING);
            polarity = 1;
        } else {
            // secondary address
            attachInterruptArg(digitalPinToInterrupt(dev->irq_pin), irq_handler, (void *)dev, FALLING);
            polarity = 0;
        }
        log_i("dev=%d addr=0x%x irq pin=%u polarity=%u", i, dev->i2caddr, dev->irq_pin, polarity);

        // identify the device
        log_i("DPS3xx: product 0x%x revision 0x%x",
              dps->getProductId(),
              dps->getRevisionId());

        // measure temperature once for pressure compensation
        float temperature;
        if ((ret = dps->measureTempOnce(temperature, dev->temp_osr)) != 0) {
            log_e("measureTempOnce failed ret=%d", ret);
        } else {
            log_i("dps3xx compensating for %.2f°", temperature);
        }

        // define what causes an interrupt: both temperature and pressure conversion
        // FIXME polarity
        if ((ret = dps->setInterruptSources(DPS3xx_BOTH_INTR, polarity)) != 0) {
            log_i("setInterruptSources: %d", ret);
        }

        // clear interrupt flags by reading the IRQ status register
        if ((ret = dps->getIntStatusPrsReady()) != 0) {
            log_i("getIntStatusPrsReady: %d", ret);
        }

        // start one-shot conversion
        // interrupt will be posted at end of conversion
        if ((ret = dps->startMeasurePressureOnce(dev->prs_osr)) != 0) {
            log_e("startMeasurePressureOnce: %d", ret);
        }

        dev->initialized = true;
        num_devs++;
    }
    return num_devs;
}


// fetch a baro sample from the baro queue
// return false if no sample present
bool get_baro_sample(baroSample_t &s) {
    size_t sz = 0;
    baroSample_t *bs = (baroSample_t *)baro_queue->receive(&sz, 0);
    if (bs != nullptr) {
        s = *bs;
        baro_queue->return_item(bs);
        return true;
    }
    return false;
}

#define MAX_SENSORS 4
static HKalF ekfs[MAX_SENSORS];  // per-device Kalman filter
TimerStats kalman_step;

void kalman_stepz_error(double x0, double x1, double x2)  {
    log_e("step error  x0=%f x1=%f x2=%f", x0, x1, x2);
}


void baro_loop(void) {
    if (irq_queue_full || baro_queue_full || commit_fail) {
        log_e("irq_queue_full=%d baro_queue_full=%d commit_fail=%d", irq_queue_full, baro_queue_full, commit_fail);
        irq_queue_full = 0;
        baro_queue_full = 0;
        commit_fail = 0;
    }

    baroSample_t s;
    if (get_baro_sample(s)) {
        // a sample from the baro sensor queue is available.
        // log_i("dev_id=%u type=%u ts=%.3f value=%.2f", s.dev_id, s.type, s.timestamp, s.value);

        if (s.type == SAMPLE_PRESSURE) {
            // it's a pressure sample
            dps_sensors_t *dev = s.dev;      // device the sample came from
            HKalF *ekf = &ekfs[dev->dev_id]; // per-device EKF instance

            float alt = altitude_from_pressure(s.value);
            float timestamp_sec = s.timestamp * 1.0e-6; // sensor time of sample

            // initialize first time around
            if (dev->previous_time < 0.0) {
                dev->previous_alt = alt;
                dev->previous_time = timestamp_sec;
                return;
            }
            if (dev->previous_alt < 0) {
                dev->previous_alt = alt;
                return;
            }
            // now dev->previous_alt and dev->previous_time are valid

            // compute vertical speed
            float delta_alt = alt - dev->previous_alt;           // meter
            float delta_t = timestamp_sec - dev->previous_time;  // sec
            float vertical_speed = delta_alt/delta_t;       // ms/s

            // prime the altitude variance
            if (dev->initial_alt_values > 0) {
                log_d("prime(alt=%.2f, vspeed=%.2f)", alt, vertical_speed);
                ekf->prime(alt, vertical_speed);
                dev->initial_alt_values--;
                return;
            }
            // log_i("ts %f p %f alt %f vs %f",timestamp_sec, s.value, alt, vertical_speed);

            kalman_step.Start();
            bool success = ekf->accKalman(timestamp_sec, s.value, alt, vertical_speed);
            kalman_step.Stop();

            if (success) {

                JsonDocument json;
                json["time"] = timestamp_sec;
                json["hPa"] = s.value;
                json["altitude"] = alt;
                json["verticalSpeedRaw"] = vertical_speed;
                json["verticalSpeedKF"] = ekf->verticalSpeed();
                json["verticalAccelerationKF"] = ekf->verticalAcceleration();
                json["altitudeVariance"] = ekf->altitudeVariance();
                json["verticalSpeedVariance"] = ekf->verticalSpeedVariance();

                auto publish = mqtt.begin_publish(dev->topic, measureJson(json));
                serializeJson(json, publish);
                publish.send();
#if 0
                log_i("t=%.1f pressure=%.2f v_baro=%.2f a_baro=%.2f altVar=%.3f varioVar=%.3f kalman_step=%d uS",
                      ekf->timeOfLastStep(),
                      ekf->pressureOfLastStep(),
                      ekf->verticalSpeed(),
                      ekf->verticalAcceleration(),
                      ekf->altitudeVariance(),
                      ekf->verticalSpeedVariance(),
                      (int32_t) kalman_step.Mean());
#endif
            } else {
                kalman_stepz_error(ekf->getX(0), ekf->getX(1), ekf->getX(2));
            }
        } else {
            // a temperature sample - unused
        }
    }
#if 1



#endif
}