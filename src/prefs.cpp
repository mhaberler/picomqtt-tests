#include "prefs.hpp"
#include "broker.hpp"
#include "tickers.hpp"


void settings_tick() {
    imu_settings.publish();
    baro_settings.publish();
    gps_settings.publish();
}

void  settings_setup() {
    imu_settings.begin();
    baro_settings.begin();
    gps_settings.begin();
}
