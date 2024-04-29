#include <Arduino.h>
#include <NimBLEDevice.h>

#define round1(x) (round((x)*1.0e1) / 1.0e1)
#define round2(x) (round((x)*1.0e2) / 1.0e2)
#define round3(x) (round((x)*1.0e3) / 1.0e3)
#define round4(x) (round((x)*1.0e4) / 1.0e4)
#define round5(x) (round((x)*1.0e5) / 1.0e5)
#define round6(x) (round((x)*1.0e6) / 1.0e6)

uint8_t volt2percent(const float volt);
const String bleTopic(const NimBLEAddress &mac);
