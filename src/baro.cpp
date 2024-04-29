#include <Arduino.h>
#include "i2cio.hpp"
#include <Dps3xx.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "baro.hpp"
#include "broker.hpp"
#include "stats.hpp"
#include "meteo.hpp"

dps_sensors_t dps_sensors[] = {
    {
        0, 0, NULL, false, &Wire, 0x77, "dps368-0",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
    {
        0, 0,  NULL, false, &Wire, 0x76, "dps368-1",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
    {
        0, 0,  NULL, false, &Wire1, 0x77, "dps368-2",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    }
};

bool dps368_setup(int i) {
    dps_sensors_t *d = &dps_sensors[i];
    if (i2c_probe(*d->wire, d->i2caddr)) {
        Dps3xx *sensor = new Dps3xx();
        sensor->begin(Wire, d->i2caddr);
        log_i("%s: product 0x%x revision 0x%x", d->topic, sensor->getProductId(), sensor->getRevisionId());
        sensor->standby();
        d->prs_tick = d->temp_tick = micros();
        int16_t ret = sensor->startMeasureBothCont(d->temp_mr, d->temp_osr, d->prs_mr, d->prs_osr);

        if (ret != 0) {
            log_e("startMeasureBothCont %s failed ret=%d", d->topic, ret);
            delete sensor;
            d->initialized = false;
            return false;
        }
        d->initialized = true;
        d->sensor = sensor;
        return true;
    }
    return false;
}

void baro_loop(void) {
    for (auto i = 0; i < 3; i++) {
        dps_sensors_t *d = &dps_sensors[i];
        if (!d->initialized)
            continue;

        int16_t val =  d->sensor->getIntStatusFifoFull();

        uint8_t pressureCount = 20;
        float pressure[pressureCount];
        uint8_t temperatureCount = 20;
        float temperature[temperatureCount];

        int16_t ret = d->sensor->getContResults(temperature, temperatureCount, pressure, pressureCount);
        unsigned long now = micros();

        // log_d("tcount %u pcount %u",  temperatureCount, pressureCount);

        JsonDocument json;
        json["tick"] = micros();

        if (temperatureCount) {
            if (d->temp_cnt == 0) { // first reading
                d->temp_smoothed = temperature[0];
                d->temp_tick = now;
            } else {
                // IIR filter samples
                for (auto i = 0; i < temperatureCount; i++) {
                    d->temp_smoothed = d->temp_alpha * temperature[i] +
                                       (1.0 - d->temp_alpha) * d->temp_smoothed;
                }
            }
            d->temp_cnt += temperatureCount;
#ifdef REPORT_BARO_TEMPERATURE
            json["temp"] = d->temp_smoothed;
#else
            temperatureCount = 0;
#endif
        }
        if (pressureCount) {
            if (d->prs_cnt == 0) { // first reading
                d->prs_smoothed = pressure[0];
                d->prs_tick = now;
            } else {
                // IIR filter samples
                for (auto i = 0; i < pressureCount; i++) {
                    d->prs_smoothed = d->prs_alpha * pressure[i] +
                                      (1.0 - d->prs_alpha) * d->prs_smoothed;
#ifdef STATS
                    alt_stats.Push(hPa2meters(pressure[i] / 100.0));
#endif
                }
            }
            d->prs_cnt += pressureCount;
            float hPa = d->prs_smoothed  / 100.0;
            json["hPa"] = hPa;
            json["alt"] = hPa2meters(hPa);
            json["last-alt"] = hPa2meters(pressure[pressureCount-1] / 100.0);
            json["last-hPa"] = pressure[pressureCount-1] / 100.0; // last raw sample
        }
        if (pressureCount || temperatureCount) {
            auto publish = mqtt.begin_publish(d->topic, measureJson(json));
            serializeJson(json, publish);
            publish.send();
        }
    }
}

uint8_t baro_setup(void) {
    uint8_t dps_count = 0;
    for (auto i = 0; i < 3; i++) {
        if (dps368_setup(i))
            dps_count++;
    }
    return dps_count;
}
