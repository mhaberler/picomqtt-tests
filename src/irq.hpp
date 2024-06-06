#pragma once
#include "stdint.h"
#include "esp_attr.h"
#include "ringbuffer.hpp"

typedef struct {
    float timestamp;
    void *dev;
} irqmsg_t;

extern espidf::RingBuffer *measurements_queue;

extern uint32_t irq_queue_full, measurements_queue_full, commit_fail;

void IRAM_ATTR  irq_handler(void *param);
void soft_irq(void* arg);
