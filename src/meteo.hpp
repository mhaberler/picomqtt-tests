#pragma once
#include <cmath>

#define SEALEVELPRESSURE_HPA (1013.25)

static inline float hPa2meters(float hPa, float seaLevel = SEALEVELPRESSURE_HPA) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  return 44330.0 * (1.0 - pow(hPa / seaLevel, 0.1903));
}
