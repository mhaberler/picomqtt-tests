
#ifdef M5UNIFIED
    #include <M5Unified.h>
#else
    #include <Arduino.h>
#endif
#include <ArduinoJson.h>
#include "fmicro.h"
#include "broker.hpp"

void battery_check(void) {
#ifdef M5UNIFIED
        JsonDocument json;
        json["time"] = fseconds();
        json["level"] = M5.Power.getBatteryLevel();
        json["status"] = (int) M5.Power.isCharging();
        // these values report nonsense with M5Unified 1.16
        // json["mV"] = (int) M5.Power.getBatteryVoltage();
        // json["mA"] = (int) M5.Power.getBatteryCurrent();

        switch (M5.Power.isCharging()) {
            case m5::Power_Class::is_discharging:
                json["text"]  = "discharging";
                break;
            case m5::Power_Class::is_charging:
                json["text"]  = "charging";
                break;
            case m5::Power_Class::charge_unknown:
                json["text"]  = "unknown";
                break;
        }
        auto publish = mqtt.begin_publish("system/battery", measureJson(json));
        serializeJson(json, publish);
        publish.send();
#endif
}
