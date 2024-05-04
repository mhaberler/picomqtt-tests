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
#include "hKalF_acc.h"

dps_sensors_t dps_sensors[] = {
#ifdef DPS0
    {
        0, 0, NULL, false, &Wire, 0x77, "dps368-0",
        TEMP_MEASURE_RATE,
        TEMP_OVERSAMPLING_RATE,
        PRS_MEASURE_RATE,
        PRS_OVERSAMPLING_RATE,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
#endif
#ifdef DPS1
    {
        0, 0,  NULL, false, &Wire, 0x76, "dps368-1",
        TEMP_MEASURE_RATE,
        TEMP_OVERSAMPLING_RATE,
        PRS_MEASURE_RATE,
        PRS_OVERSAMPLING_RATE,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
#endif
#ifdef DPS2
    {
        0, 0,  NULL, false, &Wire1, 0x77, "dps368-2",
        TEMP_MEASURE_RATE,
        TEMP_OVERSAMPLING_RATE,
        PRS_MEASURE_RATE,
        PRS_OVERSAMPLING_RATE,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
#endif
#ifdef DPS3
    {
        0, 0,  NULL, false, &Wire1, 0x76, "dps368-3",
        TEMP_MEASURE_RATE,
        TEMP_OVERSAMPLING_RATE,
        PRS_MEASURE_RATE,
        PRS_OVERSAMPLING_RATE,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    }
#endif
};
#define NUM_DPS (sizeof(dps_sensors)/sizeof(dps_sensors[0]))

HKalF ekf[NUM_DPS];

bool dps368_setup(int i) {
    dps_sensors_t *d = &dps_sensors[i];
    if (i2c_probe(*d->wire, d->i2caddr)) {
        Dps3xx *sensor = new Dps3xx();
        sensor->begin(*d->wire, d->i2caddr);
        log_i("%s: product 0x%x revision 0x%x", d->topic, sensor->getProductId(), sensor->getRevisionId());
        float temperature;
        int16_t ret = sensor->measureTempOnce(temperature, 7);
        log_i("%s: calibrate at %.2f°", d->topic, temperature);
        sensor->standby();
        d->initialized = true;
        d->sensor = sensor;
        return true;
    }
    return false;
}

void baro_loop(void) {
    for (auto i = 0; i < NUM_DPS; i++) {
        dps_sensors_t *d = &dps_sensors[i];
        if (!d->initialized)
            continue;
        float pressure;
        int16_t val =  d->sensor->measurePressureOnce(pressure, d->prs_osr);
        uint32_t now = millis();

        JsonDocument json;
        json["tick"] = now;
        float hPa = pressure  / 100.0;
        json["hPa"] = hPa;
        float altitude = Get_QNH_Altitude(hPa, QNH);
        json["alt"] = altitude;

        if (d->previous_altitude > 0.0) {
            float delta_t = (now - d->prs_tick) /1000.0;  // sec
            float delta_alt = d->previous_altitude - altitude;
            float vertical_speed = delta_alt/delta_t;       // ms/s
            json["vspeed"] = vertical_speed;

            if (ekf[i].accKalman(now/1000.0, pressure, altitude, vertical_speed)) {
                json["v_baro"] = ekf[i].verticalSpeed();
                json["a_baro"]  =  ekf[i].verticalAcceleration();
            }

        }
        d->previous_altitude = altitude;
        d->prs_tick = now;
        auto publish = mqtt.begin_publish(d->topic, measureJson(json));
        serializeJson(json, publish);
        publish.send();
    }
}

uint8_t baro_setup(void) {
    uint8_t dps_count = 0;
    for (auto i = 0; i < NUM_DPS; i++) {
        if (dps368_setup(i))
            dps_count++;
    }
    return dps_count;
}
