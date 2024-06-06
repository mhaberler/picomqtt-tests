
#include "irq.hpp"
#include "esp32-hal.h"
#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "i2cio.hpp"
#include "pindefs.h"

#define MAX_SENSORS 3
#ifdef EKF
    static HKalF ekfs[MAX_SENSORS];  // per-device Kalman filter
#endif

bool dps368_irq(dps_sensors_t * dev, const float &timestamp) {
    float value;
    int16_t ret, ret2 ;
    bool result = true;

    if ((ret = dev->sensor->getSingleResult(value)) != 0) {
        log_e("%s: getSingleResult: %d", dev->dev.topic, ret);
        TOGGLE(TRIGGER1);
        if (ret != DPS__SUCCEEDED) {
            if ((ret2 = dev->sensor->standby()) != DPS__SUCCEEDED) {
                log_e("%s: standby: %d", dev->dev.topic, ret2);
            }
        }
        result = false;
    }

    if (((ret = dev->sensor->getIntStatusPrsReady()) < 0)) {
        log_e("%s: getIntStatusPrsReady: %d", dev->dev.topic, ret);
        TOGGLE(TRIGGER1);
        result = false;
    }

    if (result) {
        // a pressure sample is ready
        baroSample_t *bs = nullptr;
        size_t sz = sizeof(baroSample_t);
        if (measurements_queue->send_acquire((void **)&bs, sz, 0) != pdTRUE) {
            measurements_queue_full++;
            TOGGLE(TRIGGER2);
        } else  {
            bs->dev = dev;
            bs->timestamp = timestamp;
            if (ret == 1) {
                bs->type  = SAMPLE_PRESSURE;
                bs->value = value / 100.0f;  // scale to hPa
            } else {
                bs->type  = SAMPLE_TEMPERATURE;
                bs->value = value; // already degC
            }
            if (measurements_queue->send_complete(bs) != pdTRUE) {
                commit_fail++;
            }
        }
    }

    // start a new measurement cycle
    // every count & TEMP_COUNT_MASK pressure measurements start a temperature measurement
    // to keep correction accurate
    dev->dev.softirq_count++;
    if ((dev->dev.softirq_count & dev->temp_measure_mask)) {
        if ((ret = dev->sensor->startMeasurePressureOnce(dev->prs_osr)) != 0) {
            log_e("%s: startMeasurePressureOnce: %d", dev->dev.topic, ret);
            TOGGLE(TRIGGER1);
            return result;
        }
    } else {
        if ((ret = dev->sensor->startMeasureTempOnce(dev->temp_osr)) != 0) {
            log_e("%s: startMeasureTempOnce: %d", dev->dev.topic, ret);
            TOGGLE(TRIGGER1);
            return result;
        }
    }
    return true;
}

static void dps368_deconfigure(const char *tag, dps_sensors_t *dev) {
    int16_t ret;
    if (dev->dev.irq_attached) {
        log_e("%s: %s - detaching interrupt", tag, dev->dev.topic);
        detachInterrupt(digitalPinToInterrupt(dev->dev.irq_pin));
        dev->dev.irq_attached = false;
    }
    if (dev->sensor) {
        // if ((ret = dev->sensor->softReset()) != 0) {
        //     log_e("%s: softReset: %d", dev->dev.topic, ret);
        // }
        // delay(100);
        log_e("%s: %s - sensor end()", tag, dev->dev.topic);
        dev->sensor->end();
        delay(100);
    }
    dev->dev.device_initialized = false;
}

int16_t dps368_setup(int i) {
    int16_t ret;
    dps_sensors_t *dev = &dps_sensors[i];
    dev->dev.init_count++;

    if (!detect(*dev->wire, dev->dev.i2caddr)) {
        log_e("device %s not present on Wire%u 0x%x",
              dev->dev.topic, dev->wire == &Wire ? 0 : 1, dev->dev.i2caddr);
        dev->dev.device_initialized = false;
        return DPS__DEVICE_NOT_RESPONDING;
    }
    dev->dev.device_present = true;
    log_e("%s: init #%d", dev->dev.topic, dev->dev.init_count);

    dps368_deconfigure("setup", dev);

    // interrupts detached
    // Dps3xx instance ended or nonexistent
    if (!dev->sensor) {
        dev->sensor = new Dps3xx(); // first setup
    }
    // Dps3xx instance exists

    log_i("init %s bus=%d addr=0x%x irq_pin=%u irq_pin_mode=%u edge=%d polarity=%u", dev->dev.topic,
          dev->wire == &Wire ? 0:1,
          dev->dev.i2caddr, dev->dev.irq_pin, dev->dev.irq_pin_mode,
          dev->dev.irq_pin_edge, dev->irq_polarity);

    pinMode(dev->dev.irq_pin, dev->dev.irq_pin_mode);

    dev->sensor->begin(*dev->wire, dev->dev.i2caddr);
    delay(100);
    if ((ret = dev->sensor->standby()) != DPS__SUCCEEDED) {
        TOGGLE(TRIGGER1);
        log_e("%s: standby failed: %d", dev->dev.topic, ret);
        goto FAIL;
    }

    // identify the device
    log_i("%s: product 0x%x revision 0x%x", dev->dev.topic,
          dev->sensor->getProductId(),
          dev->sensor->getRevisionId());

    // measure temperature once for pressure compensation
    float temperature;
    if ((ret = dev->sensor->measureTempOnce(temperature, dev->temp_osr)) != 0) {
        log_e("%s: measureTempOnce failed ret=%d", dev->dev.topic, ret);
        TOGGLE(TRIGGER1);
        goto FAIL;
    } else {
        log_i("%s: compensating for %.2f°", dev->dev.topic, temperature);
    }

    if ((ret = dev->sensor->standby()) != DPS__SUCCEEDED) {
        log_e("%s: standby2 failed: %d", dev->dev.topic, ret);
        TOGGLE(TRIGGER1);
        goto FAIL;
    }

    attachInterruptArg(digitalPinToInterrupt(dev->dev.irq_pin), irq_handler, (void *)dev, dev->dev.irq_pin_edge);
    dev->dev.irq_attached = true;

    // define what causes an interrupt: both temperature and pressure conversion
    if ((ret = dev->sensor->setInterruptSources(DPS3xx_BOTH_INTR, dev->irq_polarity)) != 0) {
        log_i("%s: setInterruptSources: %d", dev->dev.topic, ret);
        goto FAIL;
    }
    // clear interrupt flags by reading the IRQ status register
    if ((ret = dev->sensor->getIntStatusPrsReady()) != 0) {
        log_i("%s: getIntStatusPrsReady: IRQ pending", dev->dev.topic, ret);
        // TOGGLE(TRIGGER1);
    }
    // start one-shot conversion
    // interrupt will be posted at end of conversion
    if ((ret = dev->sensor->startMeasurePressureOnce(dev->prs_osr)) != 0) {
        log_e("%s: startMeasurePressureOnce: %d", dev->dev.topic, ret);
        TOGGLE(TRIGGER1);
        goto FAIL;
    }
    dev->dev.device_initialized = true;
#ifdef EKF
    dev->ekf = &ekfs[i];  // per-device Kalman filter
#endif
    log_e("%s: init %d success", dev->dev.topic, dev->dev.init_count);
    return DPS__SUCCEEDED;

FAIL:
    dps368_deconfigure("fail", dev);

    // ret != DPS__SUCCEEDED
    // device_initialized == false
    // irq_attached == false
    return ret;
}
