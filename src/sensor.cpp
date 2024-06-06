#include <Arduino.h>
#include "sensor.hpp"
#include "i2cio.hpp"
#include "scanner.hpp"


#include <ICM_20948.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "broker.hpp"
#include "RunningStats.hpp"
#include "meteo.hpp"

TICKER(gps, GPS_INTERVAL);
TICKER(stats, STATS_INTERVAL);
TICKER(ble, BLE_INTERVAL);

void stats_loop(void);
void stats_setup(void);

void sensor_loop(void) {
    process_measurements();

    if (TIME_FOR(gps)) {
        ublox_loop();
        DONE_WITH(gps);
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
    for (auto i = 0; i < num_dps_sensors; i++) {
        int n = 3;
        int16_t ret;
        while (n--) {
            ret = dps368_setup(i);
            if (ret == DPS__SUCCEEDED) {
                break;
            }
            log_e("dps%d setup failed ret=%d - retrying", i, ret);
            delay(200);
        }
        if (ret != DPS__SUCCEEDED) {
            log_e("dps%d setup failed ret=%d - giving up", i, ret);
        }
    }
#ifdef UBLOX_SUPPORT
    if (ublox_setup()) {
        log_i("ublox initialized");
        RUNTICKER(gps);
    }
#endif
#ifdef IMU_SUPPORT
    if (imu_setup(&imu_sensor)) {
        log_i("imu initialized");
    }
#endif
    stats_setup();
    RUNTICKER(stats);
}