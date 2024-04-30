/* Copyright_License {


  Copyright (C) 2006-2021 www.hlballon.com
  Version 1.0 210713 Hilmar Lorenz

  based on: TinyEKF: Extended Kalman Filter for Arduino and TeensyBoard.
  https://github.com/simondlevy/TinyEKF

  TinyEKF is a simple, lightweight C/C++ implementation of the Extended Kalman Filter. In order to make it practical
  for running on Arduino, STM32, M5Stack and other microcontrollers, it uses static (compile-time) memory allocations
  (no "new" or "malloc").  (C) 2015 Simon D. Levy - MIT License


  The fundamentals of our implementation for a HotAirBallooning Burner Control System (HLBc) are described on

  http://www.hlballon.com/brennersteuerung.php


  The system realizes the barometric acceleration measurement and provides a direct estimation of the
  vertical forces applied by external impulses (burner, meteorological influences) to the balloon.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
 */

#ifndef H_KALF_ACC_H
#define H_KALF_ACC_H
#include <TinyEKF.h>

class HKalF : public TinyEKF {

  public:

    HKalF() {
        // We approximate the process noise using a small constant
        this->setQ(0, 0, 1);
        this->setQ(1, 1, 0.1);
        this->setQ(2, 2, 0.01);

        this->setP(0, 0, 0.05);
        this->setP(1, 1, 0.001);
        this->setP(2, 2, 0.001);
    }

    void setR_M(int i, int j, double value) {
        this->setR(i, j, value);
    }

  protected:
    void model(double fx[Nsta], double F[Nsta][Nsta], double hx[Mobs], double H[Mobs][Nsta]) {

        fx[0] = this->x[0] + this->x[1] + 0.5* this->x[2];
        fx[1] = this->x[1] + this->x[2];
        fx[2] = this->x[2];

        F[0][0] = 1;
        F[0][1] = 1;
        F[0][2] = 0.5;
        F[1][0] = 0;
        F[1][1] = 1;
        F[1][2] = 1;
        F[2][0] = 0;
        F[2][1] = 0;
        F[2][2] = 1;

        hx[0] = this->x[0]; // Altitude from previous state

        // Jacobian of the measurement function
        H[0][0] = 1;        // Altitude from previous state
    }
};

extern float infoStatistics(float f_Time[11], float f_Elv[11]);
extern void init_Kalman(uint32_t now, float initial_altitude);
extern void accKalman(unsigned long t, float pressure, float altitude, float vertical_speed);
extern void kalman_stepz_error(double x0, double x1, double x2);
extern void kalman_report(bool ok, float pressure, float v_baro,float a_baro, double altVar, double  varioVar);

#endif
