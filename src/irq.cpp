#include "irq.hpp"
#include "esp32-hal.h"
#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "broker.hpp"
#include "tickers.hpp"
#include "fmicro.h"
#include "pindefs.h"
#undef TRACE_PINS

void ublox_read(const void *dev);
void nfc_poll(void);
void battery_check(void);

QueueHandle_t irq_queue;
espidf::RingBuffer *measurements_queue;
TaskHandle_t softirq_task;
EXT_TICKER(gps);
EXT_TICKER(deadman);

PicoSettings baro_settings(mqtt, "baro");

// lost interrupts:
// if we have not heard from a DPS3xx in i2c_timeout seconds, re-init device
float_setting_t i2c_timeout(baro_settings, "i2c_timeout", 10.0);


uint32_t hardirq_fail, softirq_fail, measurements_queue_full, commit_fail;

bool dps368_irq(dps_sensors_t * dev, const float &timestamp);
bool icm20948_irq(icm20948_t *dev, const float &timestamp);

void setup_queues(void) {
    irq_queue = xQueueCreate(IRQ_QUEUELEN, sizeof(irqmsg_t));
    measurements_queue = new espidf::RingBuffer();
    measurements_queue->create(MEASMT_QUEUELEN, RINGBUF_TYPE_NOSPLIT);
}

BaseType_t irq_run_softirq_task(void) {
#ifdef CONFIG_FREERTOS_UNICORE
    return xTaskCreate(soft_irq, "soft_irq", SOFTIRQ_STACKSIZE, NULL,
                       SOFTIRQ_PRIORITY, &softirq_task);
#else
    return xTaskCreatePinnedToCore(soft_irq, "soft_irq", SOFTIRQ_STACKSIZE, NULL,
                                   SOFTIRQ_PRIORITY, &softirq_task, 1);
#endif

}

// first level interrupt handler
// only notify 2nd level handler task passing any parameters
// call only from interruot context
void irq_handler(void *param) {
    TOGGLE(TRIGGER1);
    irqmsg_t msg;
    msg.dev = param;
    msg.timestamp = fseconds();
    if (xQueueSendFromISR(irq_queue, (const void*) &msg, NULL) != pdTRUE) {
        // should clear IRQ so as to not get stuck
        // arduino-esp32 wont permit i2c i/o here
        hardirq_fail++;
    }
    TOGGLE(TRIGGER1);
}

// post a soft irq to the handler queue/task
// call only from userland context
void post_softirq(void *dev) {
    TOGGLE(TRIGGER1);
    irqmsg_t msg = {
        .timestamp = fseconds(),
        .dev = dev
    };
    if (xQueueSend(irq_queue, (const void*) &msg, 0) != pdTRUE) {
        softirq_fail++;
    }
    TOGGLE(TRIGGER1);
}

// 2nd level interrupt handler
// runs in user context - can do Wire I/O, log etc
void soft_irq(void* arg) {
    irqmsg_t msg;

    for (;;) {
        while (xQueueReceive(irq_queue, &msg, 10 * portTICK_PERIOD_MS) == pdTRUE) {
            TOGGLE(TRIGGER2);

            devhdr_t *dh = static_cast<devhdr_t *>(msg.dev);
            i2c_gendev_t *gd = &dh->dev;
            gd->last_heard = msg.timestamp;

            switch (gd->type) {
                case DEV_DPS368: {
                        dps368_irq(static_cast<dps_sensors_t *>(msg.dev), msg.timestamp);
                    }
                    break;
#if defined(IMU_SUPPORT)

                case DEV_ICM_20948: {
                        icm20948_irq(static_cast<icm20948_t *>(msg.dev), msg.timestamp);
                    }
                    break;
#endif
                case DEV_M5STACK_IMU:
                    break;
                case DEV_NEO_M9N:
#ifdef UBLOX_SUPPORT
                    ublox_read(gd);
#endif
                    break;

                case DEV_MFRC522:
                    TOGGLE(TRIGGER2);
                    nfc_poll();
                    TOGGLE(TRIGGER2);
                    break;

                case DEV_BATTERY:
                    battery_check();
                    break;

                case DEV_MICROPHONE:
                    break;
            }
            if (uxQueueMessagesWaiting(irq_queue) == 0) {
                break;
            }
        }
        TOGGLE(TRIGGER2);

        yield();

        // check sensors being alive
        if (TIME_FOR(deadman)) {
            float now = fseconds();
            for (auto i = 0; i < num_dps_sensors; i++) {
                dps_sensors_t *d = &dps_sensors[i];
                if (!d->dev.device_present)
                    continue;
                // if (!d->dev.device_initialized)
                //     continue;
                if (now - d->dev.last_heard > i2c_timeout) {
                    log_e("%s timeout (%f sec) - reinit", d->dev.topic, i2c_timeout.get());
                    // might publish a sensor fault message here
                    int16_t ret = dps368_setup(i);
                    d->previous_alt = -1e6;
                    d->previous_time = -1e6;
                    d->initial_alt_values = INITIAL_ALTITUDE_VALUES;
                    d->dev.last_heard = now; // leave some time till next kick
                }
            }
#if defined(IMU_SUPPORT)

            if (imu_sensor.dev.device_present &&
                    // !imu_sensor.dev.device_initialized &&
                    (now - imu_sensor.dev.last_heard > i2c_timeout.get())) {
                log_e("%s timeout (%f sec) - reinit", imu_sensor.dev.topic, i2c_timeout.get());
                imu_setup(&imu_sensor);
                imu_sensor.dev.last_heard = now; // leave some time till next kick
            }
#endif
            DONE_WITH(deadman);
        }

    }

}


