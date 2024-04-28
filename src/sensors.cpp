#include <Arduino.h>
#include "sensors.hpp"
#include "i2cio.hpp"
#include <Dps3xx.h>
#include <ICM_20948.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "ublox.hpp"
#include "meteo.hpp"
#include "RunningStats.hpp"

#define TEMP_ALPHA 0.1
#define PRS_ALPHA 0.8
#define TEMP_MR   DPS__MEASUREMENT_RATE_1
#define TEMP_OSR  DPS__OVERSAMPLING_RATE_128
#define PRS_MR    DPS__MEASUREMENT_RATE_2
#define PRS_OSR   DPS__OVERSAMPLING_RATE_1

#define detect(wire, addr) (i2c_probe(wire, addr) ? addr : 0)

typedef struct  {
    uint32_t temp_cnt, prs_cnt;
    Dps3xx *sensor;
    bool initialized;
    TwoWire *wire;
    uint8_t i2caddr;
    const char *topic;
    int16_t temp_mr;
    int16_t temp_osr;
    int16_t prs_mr;
    int16_t prs_osr;
    unsigned long temp_tick, prs_tick;
    float temp_alpha, prs_alpha;
    float temp_smoothed, prs_smoothed;
} dps_sensors_t;


dps_sensors_t dps_sensors[] = {
    {
        0, 0, NULL, false, &Wire, 0x77, "dps368-1",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
    {
        0, 0,  NULL, false, &Wire, 0x76, "dps368-2",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    },
    {
        0, 0,  NULL, false, &Wire1, 0x77, "dps368-3",
        TEMP_MR,
        TEMP_OSR,
        PRS_MR,
        PRS_OSR,
        0,0,
        TEMP_ALPHA,
        PRS_ALPHA
    }
};

TICKER(dps, I2C_INTERVAL);
TICKER(gps, I2C_INTERVAL);
TICKER(imu, I2C_INTERVAL);
TICKER(stats, STATS_INTERVAL);

static uint8_t ublox_addr, icm_addr; // zero if not present
static uint8_t dps_count; // count of DPS3xx detected

static uint8_t temperature_initial_oversampling = DPS__OVERSAMPLING_RATE_128;
static uint8_t temperature_oversampling = DPS__OVERSAMPLING_RATE_1;
static uint8_t pressure_oversampling = DPS__MEASUREMENT_RATE_8;

extern PicoMQTT::Server mqtt;
RunningStats alt_stats;

bool dps368_setup(int i) {
    dps_sensors_t *d = &dps_sensors[i];
    if (i2c_probe(*d->wire, d->i2caddr)) {
        Dps3xx *sensor = new Dps3xx();
        sensor->begin(Wire, d->i2caddr);

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

void sensor_loop(void) {
    if (TIME_FOR(dps)) {
        for (auto i = 0; i < 3; i++) {
            dps_sensors_t *d = &dps_sensors[i];
            if (!d->initialized)
                continue;

            uint8_t pressureCount = 20;
            float pressure[pressureCount];
            uint8_t temperatureCount = 20;
            float temperature[temperatureCount];

            int16_t ret = d->sensor->getContResults(temperature, temperatureCount, pressure, pressureCount);
            unsigned long now = micros();

            // log_i("tcount %u pcount %u",  temperatureCount, pressureCount);

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
                json["temp"] = d->temp_smoothed;
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
                        alt_stats.Push(hPa2meters(pressure[i] / 100.0));
                    }
                }
                d->prs_cnt += pressureCount;
                float hPa =  d->prs_smoothed  / 100.0;
                json["hPa"] = hPa;
                json["alt"] = hPa2meters(hPa);
            }
            if (pressureCount || temperatureCount) {
                auto publish = mqtt.begin_publish(d->topic, measureJson(json));
                serializeJson(json, publish);
                publish.send();
            }
            DONE_WITH(dps);
        }

        if (TIME_FOR(gps)) {
            ublox_loop();
            DONE_WITH(gps);
        }

        if (TIME_FOR(imu)) {

            DONE_WITH(imu);
        }
        if (TIME_FOR(stats)) {
            JsonDocument json;
            json["tick"] = micros();
            json["mean"] = alt_stats.Mean();
            json["stddev"] = alt_stats.StandardDeviation();
            json["ci95"] = alt_stats.ConfidenceInterval(CI95);
            auto publish = mqtt.begin_publish("altstats", measureJson(json));
            serializeJson(json, publish);
            publish.send();
            DONE_WITH(stats);
        }
    }
}

void sensor_setup(void) {

    i2c_scan(Wire);

    for (auto i = 0; i < 3; i++) {
        if (dps368_setup(i))
            dps_count++;
    }
    ublox_addr = detect(Wire, 0x42);
    icm_addr = detect(Wire, ICM_20948_I2C_ADDR_AD1);

    if (dps_count) {
        RUNTICKER(dps);
    }
    if (ublox_addr) {
        ublox_setup();
        RUNTICKER(gps);
    }
    if (icm_addr) {
        RUNTICKER(imu);
    }
    RUNTICKER(stats);
}