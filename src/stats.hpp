#pragma once

#include "RunningStats.hpp"
#define TEMP_ALPHA 0.1

#ifdef STATS
extern RunningStats gps_stats, alt_stats;
#endif

void stats_loop(void);
void stats_setup(void);