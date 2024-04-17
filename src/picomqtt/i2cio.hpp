#pragma once
#include <Arduino.h>
#include <Wire.h>
bool i2cProbe(TwoWire &w, uint8_t addr);
void init_sensors(TwoWire &w);
void toggleTpin(void);
void dps3xx_loop(void);
void dps3xx_setup(void);