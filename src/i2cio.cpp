#include <Arduino.h>
#include <Wire.h>


bool i2c_probe(TwoWire &w, uint8_t addr) {
    w.beginTransmission(addr);
    return (w.endTransmission() == 0);
}

void i2c_scan(TwoWire &w) {
    uint8_t bus = (&w == &Wire) ? 0: 1;
    for (auto i = 0; i < 128; i++) {
        w.beginTransmission(i);
        if (w.endTransmission() == 0) {
            Serial.printf("Wire%u dev at 0x%x\n", bus, i);
        }
    }
}

