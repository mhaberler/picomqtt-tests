#pragma once
#include <Arduino.h>
#include <Wire.h>
bool i2c_probe(TwoWire &w, uint8_t addr);
void i2c_scan(TwoWire &w);


