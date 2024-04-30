#pragma once
#include <Arduino.h>
#include <Dps3xx.h>
#include "RunningStats.hpp"

#ifndef TEMP_ALPHA
#define TEMP_ALPHA 0.1
#endif
#ifndef PRS_ALPHA
#define PRS_ALPHA 0.3
#endif
#ifndef TEMP_MEASURE_RATE
#define TEMP_MEASURE_RATE   DPS__MEASUREMENT_RATE_1
#endif
#ifndef TEMP_OVERSAMPLING_RATE
#define TEMP_OVERSAMPLING_RATE  DPS__OVERSAMPLING_RATE_128
#endif
#ifndef PRS_MEASURE_RATE
#define PRS_MEASURE_RATE    DPS__MEASUREMENT_RATE_1
#endif
#ifndef PRS_OVERSAMPLING_RATE
#define PRS_OVERSAMPLING_RATE   DPS__OVERSAMPLING_RATE_1
#endif

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