#pragma once

#include "RunningStats.hpp"
#define TEMP_ALPHA 0.1

extern RunningStats gps_stats, alt_stats;

void stats_loop(void);
void stats_setup(void);