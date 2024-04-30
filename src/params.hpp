#pragma once

#define QNH                  1013.25

#define READ_DELAY_TIME   17  // why? FIXME
#define VARIO_CHECK_TIME  100 // original: 500
#define KALMAN_CHECK_TIME 300 // orignal 983 - why? FIXME

#define STARTUP_MSEC 1000  // dont use using pressure/alt values before