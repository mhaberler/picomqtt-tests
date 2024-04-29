#pragma once

#include <stdint.h>
#include <ArduinoJson.h>
#include "util.hpp"

typedef struct {
    uint8_t snum;
    float temperature;
    float pressure;
    uint8_t batpct;
    bool status;
    uint8_t location;
    int8_t rssi;
} tpmsAd_t;

typedef struct {
    uint8_t adress[6];
    uint8_t pressure[4];
    uint8_t temperature[4];
    uint8_t battery;
    uint8_t status;
    uint8_t location;
} tpms100Raw_t;

typedef struct {
    uint8_t pressure[4];
    uint8_t temperature[4];
    uint8_t battery;
    uint8_t adress[6];
} tpms172Raw_t;

void convertToJson(const tpmsAd_t & src, JsonVariant dst) {
    dst["press"] = round1(src.pressure);
    dst["temp"] = round1(src.temperature);
    dst["loc"] = src.location;
    dst["status"] = src.status;
    dst["batt"] = src.batpct;
    dst["rssi"] = src.rssi;
    // dst["tick"] = src.lastchange;
}