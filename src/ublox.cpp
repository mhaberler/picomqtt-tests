#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>
#include "RunningStats.hpp"
#include "i2cio.hpp"
#include "stats.hpp"
#include "ublox.hpp"
#include "protomap.hpp"
#include "broker.hpp"


SFE_UBLOX_GNSS myGNSS;

bool trace_ublox;
bool ublox_present;

static UBX_NAV_PVT_data_t ub_nav_pvt;
uint32_t ublox_updated;

const UBX_NAV_PVT_data_t &get_ublox_navdata(void) {
    return ub_nav_pvt;
}
uint32_t pps_irqs, extint_irqs,  prev_pps_irqs, prev_extint_irqs;

void
ublox_nav_pvt (UBX_NAV_PVT_data_t *ub) {
    ub_nav_pvt = *ub;
    ublox_updated = millis();
    // if (ub_nav_pvt.fixType)
    //     LV_LOG_USER("fix %u", ub_nav_pvt.fixType);


    JsonDocument json;
    json["us"] = micros();
    json["fixType"] =ub_nav_pvt.fixType;

    switch (ub_nav_pvt.fixType) {
        case 4:
        case 3:
            gps_stats.Push(ub_nav_pvt.hMSL / 1000.0);

            json["hMSL"] = ub_nav_pvt.hMSL / 1000.0;
            json["hAE"] = ub_nav_pvt.height / 1000.0;
            json["velD"] = ub_nav_pvt.height / 1000.0;
            json["gSpeed"] = ub_nav_pvt.gSpeed / 1000.0;
            json["headMot"] = ub_nav_pvt.headMot * 1e-5;
            json["hAcc"] = ub_nav_pvt.hAcc * 0.01;
            json["vAcc"] = ub_nav_pvt.vAcc * 0.01;
            json["pDOP"] = ub_nav_pvt.pDOP * 0.01;
            __attribute__ ((fallthrough));
        case 2:
            json["lat"] = ub_nav_pvt.lat * 1e-7;
            json["lon"] = ub_nav_pvt.lon * 1e-7;
            __attribute__ ((fallthrough));
        default:
            json["numSV"] = ub_nav_pvt.numSV;
            if (ub_nav_pvt.valid.bits.validDate) {
                json["year"] = ub_nav_pvt.year;
                json["month"] = ub_nav_pvt.month;
                json["day"] = ub_nav_pvt.day;
            }
            if (ub_nav_pvt.valid.bits.validTime) {
                json["hour"] = ub_nav_pvt.hour;
                json["min"] = ub_nav_pvt.min;
                json["sec"] = ub_nav_pvt.sec;
            }
            if (ub_nav_pvt.valid.bits.validMag) {
                json["magDec"] = ub_nav_pvt.magDec;
            }
    }
    auto publish = mqtt.begin_publish("gps", measureJson(json));
    serializeJson(json, publish);
    publish.send();
}

void ublox_loop(void) {
    // if (ublox_present) {

    myGNSS.checkUblox();
    myGNSS.checkCallbacks();
    // }
    // if (pps_irqs != prev_pps_irqs) {
    //     Serial.printf("PPS %u\n",pps_irqs);
    //     prev_pps_irqs = pps_irqs;
    // }
    // if (extint_irqs != prev_extint_irqs) {
    //     Serial.printf("EXTINT %u\n",extint_irqs);
    //     prev_extint_irqs = extint_irqs;
    // }
}


// void onPPS() {
//     // toggleTpin();
//     pps_irqs += 1;
// }

// void onINT() {
//     // toggleTpin();

//     extint_irqs += 1;
// }

bool ublox_setup(void) {

    // int16_t ppsIRQpin = 2;
    // pinMode(ppsIRQpin, INPUT);
    // attachInterrupt(digitalPinToInterrupt(ppsIRQpin), onPPS, RISING);

    // int16_t IRQpin = 7;
    // pinMode(IRQpin, INPUT);
    // attachInterrupt(digitalPinToInterrupt(IRQpin), onINT, RISING);

    if (detect(Wire, 0x42)) {

        myGNSS.begin(Wire, 0x42, 300, true);
        // myGNSS.clearAntPIO();
        // myGNSS.enableDebugging(Serial, trace_ublox);
        myGNSS.setNavigationFrequency(NAV_FREQUENCY);
        myGNSS.setAutoPVTcallbackPtr(&ublox_nav_pvt);
        log_i("ublox initialized");
        return true;
    }
    return false;
}

