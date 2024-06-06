#pragma once

#include <stdint.h>

#define NFC_MAX_MSG_SIZE 1024

typedef enum {
    BWTAG_NO_MATCH,
    BWTAG_RUUVI,
    BWTAG_PROXY_TAG,
} bwTagType_t;


