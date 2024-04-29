#pragma once
#include "stats.hpp"

#ifndef NAV_FREQUENCY
    #define NAV_FREQUENCY 1
#endif


void ublox_loop(void);
bool ublox_setup(void);
