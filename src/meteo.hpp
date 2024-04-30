#pragma once
#include <math.h>

#define QNH  (1013.25)

static inline float hPa2meters(float hPa, float seaLevel = QNH) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  return 44330.0 * (1.0 - pow(hPa / seaLevel, 0.1903));
}

static inline float Get_QNH_Altitude(float pressure, float calc_qnh) {
    // return the altitude in meters that corresponds to the given pressure in hundredths of a millibar
    float pressure_fact = pressure/calc_qnh;
    float temp_fact = (273.15+15)/0.0065;
    float air_const = 1/5.255;
    return ((1 - pow(pressure_fact, air_const))*temp_fact);
}