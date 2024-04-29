#pragma once
#include <Arduino.h>
#include <Dps3xx.h>
#include "RunningStats.hpp"
#define TEMP_ALPHA 0.1
#define PRS_ALPHA 0.3
#define TEMP_MR   DPS__MEASUREMENT_RATE_1
#define TEMP_OSR  DPS__OVERSAMPLING_RATE_128
#define PRS_MR    DPS__MEASUREMENT_RATE_1
#define PRS_OSR   DPS__OVERSAMPLING_RATE_1


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

extern dps_sensors_t dps_sensors[];
extern uint8_t dps_count; 
#ifdef STATS
extern RunningStats alt_stats;
#endif

void baro_loop(void);
uint8_t baro_setup(void);