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

const UBX_NAV_PVT_data_t &get_ublox_navdata(void) {
    return ub_nav_pvt;
}

void
ublox_nav_pvt (UBX_NAV_PVT_data_t *ub) {
    ub_nav_pvt = *ub;
    std::tm timeinfo = {};
#ifdef DEM_SUPPORT
    locInfo_t li = {};
    double ele;
    int rc = LS_INVALID;
#endif
    JsonDocument json;
    json["time"] = micros() * 1.0e-6;
    json["fixType"] = ub_nav_pvt.fixType;

    switch (ub_nav_pvt.fixType) {
        case 4:
        case 3:
#ifdef STATS
            gps_stats.Push(ub_nav_pvt.hMSL / 1000.0);
#endif
            json["hMSL"] = ub_nav_pvt.hMSL / 1000.0;
            json["hAE"] = ub_nav_pvt.height / 1000.0;
            json["velD"] = ub_nav_pvt.height / 1000.0;
            json["gSpeed"] = ub_nav_pvt.gSpeed / 1000.0;
            json["headMot"] = ub_nav_pvt.headMot * 1e-5;
            json["hAcc"] = ub_nav_pvt.hAcc * 0.01;
            json["vAcc"] = ub_nav_pvt.vAcc * 0.01;
            json["pDOP"] = ub_nav_pvt.pDOP * 0.01;
            __attribute__ ((fallthrough));
        case 2: {
                double lat = ub_nav_pvt.lat * 1e-7;
                double lon = ub_nav_pvt.lon * 1e-7;
                json["lat"] = lat;
                json["lon"] = lon;
#ifdef DEM_SUPPORT
                rc = getLocInfo(lat, lon, &li);

#endif
            }
            __attribute__ ((fallthrough));
        default:
            json["numSV"] = ub_nav_pvt.numSV;
            if (ub_nav_pvt.valid.bits.validDate) {
                json["year"] = ub_nav_pvt.year;
                json["month"] = ub_nav_pvt.month;
                json["day"] = ub_nav_pvt.day;
                timeinfo.tm_year = ub_nav_pvt.year - 1900;
                timeinfo.tm_mon = ub_nav_pvt.month - 1;
                timeinfo.tm_mday = ub_nav_pvt.day;
            }
            if (ub_nav_pvt.valid.bits.validTime) {
                json["hour"] = ub_nav_pvt.hour;
                json["min"] = ub_nav_pvt.min;
                json["sec"] = ub_nav_pvt.sec;
                timeinfo.tm_hour =  ub_nav_pvt.hour;
                timeinfo.tm_min = ub_nav_pvt.min;
                timeinfo.tm_sec = ub_nav_pvt.sec;
                timeinfo.tm_isdst = -1;
                if (ub_nav_pvt.valid.bits.validDate) {
                    std::time_t epoch_time = std::mktime(&timeinfo);
                    json["epoch"] = epoch_time; // Unix epoch, sec since 1-1-1970 UTC
                }
            }
            if (ub_nav_pvt.valid.bits.validMag) {
                json["magDec"] = ub_nav_pvt.magDec;
            }
    }
    auto publish = mqtt.begin_publish("gps/nav", measureJson(json));
    serializeJson(json, publish);
    publish.send();
    if (li.status == LS_VALID) {
        json.clear();
        json["time"] = micros() * 1.0e-6;
        json["meters"] = li.elevation;
        json["lat"] = ub_nav_pvt.lat * 1e-7;
        json["lon"] = ub_nav_pvt.lon * 1e-7;
        auto dempublish = mqtt.begin_publish("dem/elevation", measureJson(json));
        serializeJson(json, dempublish);
        dempublish.send();
    }
}

void ublox_loop(void) {
    if (gpsconf.dev.device_initialized) {
        post_softirq(&gpsconf);
    }
}

void ublox_poll(const void *dev) {
    // if (dev->dev.device_initialized) {
    if (gpsconf.dev.device_initialized) {
        ublox_neo.checkUblox();
        ublox_neo.checkCallbacks();
    }
}

bool ublox_setup(void) {

    if (detect(Wire, UBLOX_I2C_ADDR)) {
        gpsconf.wire = &Wire;
    } else if (detect(Wire1, UBLOX_I2C_ADDR)) {
        gpsconf.wire = &Wire1;
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
    log_i("ublox initialized 0x%x at Wire%u",
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