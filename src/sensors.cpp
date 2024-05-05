#include <Arduino.h>
#include "sensors.hpp"
#include "i2cio.hpp"
#include "scanner.hpp"


#include <ICM_20948.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "ublox.hpp"
#include "broker.hpp"
#include "baro.hpp"
#include "imu.hpp"
#include "RunningStats.hpp"
#include "meteo.hpp"

TICKER(gps, GPS_INTERVAL);
TICKER(imu, IMU_INTERVAL);
TICKER(stats, STATS_INTERVAL);
TICKER(ble, BLE_INTERVAL);

void sensor_loop(void) {
    baro_loop();

    if (TIME_FOR(gps)) {
        ublox_loop();
        DONE_WITH(gps);
    }
    if (TIME_FOR(imu)) {
        imu_loop();
        DONE_WITH(imu);
    }
    if (TIME_FOR(ble)) {
        process_ble();
        DONE_WITH(ble);
    }
    if (TIME_FOR(stats)) {
        stats_loop();
        DONE_WITH(stats);
    }
}

void sensor_setup(void) {
#ifdef BLE_SUPPORT
    setup_ble();
    RUNTICKER(ble);
#endif
    baro_setup();
    if (ublox_setup()) {
        RUNTICKER(gps);
    }
    if (imu_setup()) {
        RUNTICKER(imu);
    }
    stats_setup();
    RUNTICKER(stats);
}