#ifndef TIMERSTATS_H
#define TIMERSTATS_H

#include "RunningStats.hpp"
#include "esp_timer.h"

static inline double fmillis(void) {
    return (double) (esp_timer_get_time() / 1000.0L);
}

static inline double  fmicros(void) {
    return (double) esp_timer_get_time();
}

class TimerStats : public RunningStats {
  public:

    // interval timing: accumulate stats for duration between
    // pairs of Start()/Stop() calls:
    void Start() {
        _starttime = fmicros();
    }
    void Stop() {
        Push(fmicros() - _starttime);
    }

    // or use as a lap timer: measure stats for time between Lap() calls
    void Lap() {
        if (_laptime == 0) {
            _laptime = fmicros();
        } else {
            uint32_t now = fmicros();
            Push(now - _laptime);
            _laptime = now;
        }
    }
    void Clear() {
        _laptime = 0;
        RunningStats::Clear();
    }

  private:
    double _starttime;
    double _laptime;
};

#endif