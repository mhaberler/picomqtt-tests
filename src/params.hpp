#pragma once


// #define READ_DELAY_TIME   17  // why? FIXME
// #define VARIO_CHECK_TIME  100 // original: 500
// #define KALMAN_CHECK_TIME 300 // orignal 983 - why? FIXME

#define STARTUP_SEC 7.0  // dont use using pressure/alt values before

#define BARO_QUEUELEN 2048
#define IRQ_QUEUESIZE 10
#define TEMP_COUNT_MASK  7

#define INITIAL_ALTITUDE_VALUES 10
