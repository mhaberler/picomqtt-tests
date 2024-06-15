#pragma once
#include "PicoSettings.h"


typedef PicoSettings::Setting<float> float_setting_t;
typedef PicoSettings::Setting<double> double_setting_t;
typedef PicoSettings::Setting<bool> bool_setting_t;
typedef PicoSettings::Setting<int> int_setting_t;

extern PicoSettings imu_settings;
extern bool_setting_t set_reference_correction;
extern bool_setting_t apply_reference_correction;
extern double_setting_t ref_x;
extern double_setting_t ref_y;
extern double_setting_t ref_z;
extern double_setting_t ref_w;
extern double_setting_t heading_correction;
extern int_setting_t dmp_rate;
extern float_setting_t quat9_rate;

extern PicoSettings gps_settings;
extern int_setting_t nav_rate;

extern PicoSettings baro_settings;
extern float_setting_t i2c_timeout;
extern int_setting_t alpha_pct;

void settings_setup();
void settings_tick();