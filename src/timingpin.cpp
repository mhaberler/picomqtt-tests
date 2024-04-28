#include <Arduino.h>
#include "timingpin.hpp"


static bool state = false;
static int16_t timingPin;

void timingpin_toggle(void) {
    state ^= 1;
    digitalWrite(timingPin, state);
}

void timingpin_set(bool s) {
    digitalWrite(timingPin, s);
    state = s;
}

void timingpin_setup(int pin) {
    timingPin = pin;
    pinMode(timingPin, OUTPUT);
    state = false;
    digitalWrite(timingPin, state);
}

