#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "meteo.hpp"
#ifdef EKF
    #include "hKalF_acc.h"
#endif
#include "TimerStats.hpp"
#include "ArduinoJson.h"
#include "broker.hpp"


TimerStats kalman_step;

void kalman_stepz_error(double x0, double x1, double x2)  {
    log_e("step error  x0=%f x1=%f x2=%f", x0, x1, x2);
}

void process_pressure(const baroSample_t s) {
    if (s.type != SAMPLE_PRESSURE) {
        return; // TBD: use temperature
    }

    dps_sensors_t *dev = s.dev;      // device the sample came from

    float alt = altitude_from_pressure(s.value);
    float timestamp_sec = s.timestamp; // sensor time of sample

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


#ifdef EKF
    // prime the altitude variance
    if (dev->initial_alt_values > 0) {
        log_d("%s: prime(alt=%.2f, vspeed=%.2f)", dev->dev.topic, alt, vertical_speed);
        dev->ekf->prime(alt, vertical_speed);
        dev->initial_alt_values--;
        return;
    }

    // log_i("ts %f p %f alt %f vs %f",timestamp_sec, s.value, alt, vertical_speed);

    kalman_step.Start();
    bool success = dev->ekf->accKalman(timestamp_sec, s.value, alt, vertical_speed);
    kalman_step.Stop();
#else
    bool success = true;
#endif
    if (success) {

        JsonDocument json;
        json["time"] = timestamp_sec;
        json["hPa"] = s.value;
        json["altitude"] = alt;
        json["verticalSpeedRaw"] = vertical_speed;

#ifdef EKF
        json["verticalSpeedKF"] = dev->ekf->verticalSpeed();
        json["verticalAccelerationKF"] = dev->ekf->verticalAcceleration();
        json["altitudeVariance"] = dev->ekf->altitudeVariance();
        json["verticalSpeedVariance"] = dev->ekf->verticalSpeedVariance();
#endif
        auto publish = mqtt.begin_publish(dev->dev.topic, measureJson(json));
        serializeJson(json, publish);
        publish.send();
#if 0
        log_i("t=%.1f pressure=%.2f v_baro=%.2f a_baro=%.2f altVar=%.3f varioVar=%.3f kalman_step=%d uS",
              dev->ekf->timeOfLastStep(),
              dev->ekf->pressureOfLastStep(),
              dev->ekf->verticalSpeed(),
              dev->ekf->verticalAcceleration(),
              dev->ekf->altitudeVariance(),
              dev->ekf->verticalSpeedVariance(),
              (int32_t) kalman_step.Mean());
#endif
    } else {
#ifdef EKF
        kalman_stepz_error(dev->ekf->getX(0), dev->ekf->getX(1), dev->ekf->getX(2));
#endif
    }
}

void process_measurements(void) {

    if (irq_queue_full || measurements_queue_full || commit_fail) {
        log_e("irq_queue_full=%d measurements_queue_full=%d commit_fail=%d",
              irq_queue_full, measurements_queue_full, commit_fail);
        irq_queue_full = 0;
        measurements_queue_full = 0;
        commit_fail = 0;
        log_i("rbfree %u maxitem %u", measurements_queue->curr_free_size(), measurements_queue->max_item_size());

    }

    size_t sz;
    void *p;

    while ((p = measurements_queue->receive(&sz, 0)) != nullptr) {
        baroSample_t *bs = static_cast<baroSample_t *>(p);
        i2c_gendev_t *gd = &bs->dev->dev;

        switch (gd->type) {
            case DEV_DPS368: {
                    baroSample_t sample = * static_cast<baroSample_t *>(p);
                    measurements_queue->return_item(p);
                    process_pressure(sample);
                }
                break;
            case DEV_ICM_20948: {
                    imuSample_t sample = * static_cast<imuSample_t *>(p);
                    measurements_queue->return_item(p);
                    process_imu(sample);
                }
                break;
            case DEV_NEO_M9N: {
                    gpsSample_t gs = * static_cast<gpsSample_t *>(p);
                    measurements_queue->return_item(p);
                }
                break;
            // case DEV_MFRC522: { // FIXME

            //     }
            case DEV_NONE:
                measurements_queue->return_item(p);
                break;
            default:
                log_e("invalid sample type %d size %u", gd->type, sz);
                measurements_queue->return_item(p);
        }
        // yield();
    }
}