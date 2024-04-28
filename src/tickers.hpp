#pragma once

#include <Ticker.h>

#define TICKER(x, interval) \
    Ticker x##_ticker; \
    uint32_t x##_update_ms = interval; \
    bool run_##x;
#define RUNTICKER(x)  x##_ticker.attach_ms(x##_update_ms, [](void) { run_##x = true; });
#define CHANGE_TICKER(x, interval) x##_ticker.detach(); x##_update_ms = interval; RUNTICKER(x);
#define TIME_FOR(x) run_##x 
#define DONE_WITH(x) run_##x = false
#define EXT_TICKER(x) extern bool run_##x;

