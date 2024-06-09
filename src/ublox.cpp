#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "RunningStats.hpp"
#include "i2cio.hpp"
#include "stats.hpp"
#include "sensor.hpp"
#include "broker.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "tickers.hpp"
#include "fmicro.h"
#include <chrono>
#include <ctime>
#ifdef DEM_SUPPORT
    #include "demlookup.hpp"
    #include "protomap.hpp"
#endif

extern gps_sensor_t gpsconf;
PicoSettings gps_settings(mqtt, "gps");

#ifdef UBLOX_SUPPORT

SFE_UBLOX_GNSS ublox_neo;
static UBX_NAV_PVT_data_t ub_nav_pvt;

void
ublox_nav_pvt (UBX_NAV_PVT_data_t *ub) {
    gpsSample_t *gs = nullptr;
    size_t sz = sizeof(gpsSample_t);
    if (measurements_queue->send_acquire((void **)&gs, sz, 0) == pdTRUE) {
        gs->dev = &gpsconf;
        gs->timestamp = fseconds(),
        gs->type = SAMPLE_GPS;
        memcpy((void *)&gs->nav_data, ub, sizeof(UBX_NAV_PVT_data_t));
        if (measurements_queue->send_complete(gs) != pdTRUE) {
            commit_fail++;
        }
    } else {
        measurements_queue_full++;
    }
}


void ublox_trigger_read(void) {
    if (gpsconf.dev.device_initialized) {
        post_softirq(&gpsconf);
    }
}

void ublox_read(const void *dev) {
    if (gpsconf.dev.device_initialized) {
        // std::unique_lock<std::mutex> lock(ublox_mutex);
        ublox_neo.checkUblox();
        ublox_neo.checkCallbacks();
    }
}

bool ublox_setup(void) {

    if (detect(Wire, UBLOX_I2C_ADDR)) {
        gpsconf.wire = &Wire;
        gpsconf.dev.device_present = true;
        log_i("ublox at Wire 0x%x",UBLOX_I2C_ADDR );

    } else if (detect(Wire1, UBLOX_I2C_ADDR)) {
        gpsconf.wire = &Wire1;
        gpsconf.dev.device_present = true;
        log_i("ublox at Wire1 0x%x",UBLOX_I2C_ADDR );

    } else {
        log_e("no ublox device found");
        gpsconf.dev.device_initialized = false;
        return false;
    }
    ublox_neo.begin(*gpsconf.wire, gpsconf.dev.i2caddr, 300, true);
    if (gpsconf.trace) {
        ublox_neo.enableDebugging(Serial, gpsconf.trace);
    }
    ublox_neo.setNavigationFrequency(gpsconf.navFreq);
    ublox_neo.setAutoPVTcallbackPtr(&ublox_nav_pvt);
    
    log_i("ublox initialized 0x%x Wire%u",
          UBLOX_I2C_ADDR, (gpsconf.wire == &Wire) ? 0: 1);
    gpsconf.dev.device_initialized = true;
    return true;
}

EXT_TICKER(gps);
int_setting_t nav_rate(gps_settings, "nav_rate", 1, [] {
    if (gpsconf.dev.device_initialized) {
        if (nav_rate <= 0) {
            log_i("gps sampling stopped");
            STOPTICKER(gps);
        } else {
            uint32_t period = 1000/nav_rate;
            log_i("gps sampling set to %u mS", period);
            CHANGE_TICKER(gps, period);
            ublox_neo.setNavigationFrequency(nav_rate);
        }
    }
});


#else
bool ublox_setup() {
    return false;
}
void ublox_loop(void) {}
#endif