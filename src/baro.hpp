#pragma once

#include <Dps3xx.h>

#include "ringbuffer.hpp"

extern espidf::RingBuffer *baro_queue;

typedef enum {
    SAMPLE_INVALID,
    SAMPLE_PRESSURE,
    SAMPLE_TEMPERATURE,
} sample_t;


typedef struct  {
    uint8_t dev_id;
    uint8_t i2caddr;
    uint8_t irq_pin;
    uint8_t status;
    bool initialized;
    Dps3xx *sensor;
    uint32_t softirq_count;
    uint32_t temp_measure_mask;
    TwoWire *wire;
    int16_t temp_osr;
    int16_t prs_osr;
    // per-instance tracking values
    float previous_alt;
    float previous_time;
    uint32_t initial_alt_values;
    // MQTT-relate
    const char *topic;
} dps_sensors_t;

typedef struct  {
    uint8_t dev_id;
    dps_sensors_t *dev;
    sample_t type;
    float timestamp;   // uS
    float value;       // hPa/degC
} baroSample_t;

typedef struct {
    uint32_t timestamp;
    dps_sensors_t *dev;
} irqmsg_t;

bool get_baro_sample(baroSample_t &s);
int32_t baro_setup(void);
void baro_loop(void);

extern uint32_t irq_queue_full, baro_queue_full, commit_fail;

extern dps_sensors_t dps_sensors[];
extern uint32_t num_sensors;


