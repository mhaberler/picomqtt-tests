#pragma once

#include <stdint.h>
#include <ArduinoJson.h>

typedef struct {
    uint16_t status;
    uint32_t serial;
    uint16_t model;
    float level;
    int8_t rssi;
    bool has_level, has_serial;
} otodataAd_t;

void convertToJson(const otodataAd_t & src, JsonVariant dst) {
    if (src.has_level) {
        dst["level"] = src.level/100.0;
        dst["status"] = src.status;
    }
    if (src.has_serial) {
        dst["serial"] = src.serial;
        dst["model"] = src.model;
    }
    dst["rssi"] = src.rssi;
    // dst["tick"] = src.lastchange;
}
