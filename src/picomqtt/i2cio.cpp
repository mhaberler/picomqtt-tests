#include <Arduino.h>
#include <Wire.h>
void dps3xx_setup(void);

bool i2cProbe(TwoWire &w, uint8_t addr) {
    w.beginTransmission(addr);
    return (w.endTransmission() == 0);
}

bool i2cScan(TwoWire &w) {
    uint8_t bus = (&w == &Wire) ? 0: 1;
    for (auto i = 0; i < 128; i++) {
        w.beginTransmission(i);
        if (w.endTransmission() == 0) {
            Serial.printf("Wire%u dev at 0x%x\n", bus, i);
        }
    }
    return false;
}

bool tpin = false;
int16_t timingPin = 1;
void toggleTpin(void) {
    tpin ^= 1;
    digitalWrite(timingPin, tpin);
}

void init_sensors(TwoWire &w) {

    pinMode(timingPin, OUTPUT);
    digitalWrite(timingPin, tpin);
    i2cScan(w);
  
}



