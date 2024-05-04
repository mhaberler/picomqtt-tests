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

#define Nsta 3     // Three state values: alt, v, a
#define Mobs 1     // Tue measurements: alt, vario

#include <TinyEKF.h>

#include "RunningStats.hpp"

class HKalF : public TinyEKF {

  public:

    HKalF(double q0 = 1.0, double q1 = 0.1, double q2 = 0.01,
          double p0 = 0.05, double p1 = 0.001, double p2 = 0.001
         ) {

        // We approximate the process noise using a small constant
        this->setQ(0, 0, q0);
        this->setQ(1, 1, q1);
        this->setQ(2, 2, q2);

        this->setP(0, 0, p0);
        this->setP(1, 1, p1);
        this->setP(2, 2, p2);
    }

    bool accKalman(float t, float p, float altitude, float vertical_speed) {
        _time = t;
        _pressure = p;
        if (_altitude_variance.NumDataValues() == 0) {
            _altitude_variance.Push(altitude);
        }
        _altitude_variance.Push(altitude);

        // sigmaKal Faktor = 3.0 aus st_hlballon.cpp
        _altVar = 3.0 *  _altitude_variance.Variance();

        setR_M(0, 0, _altVar);
        double z  =  altitude ;

        // Indication Kalman Error - Haik
        bool ok = true;
        if (!step(&z)) {
            _step_errors++;
            return false;
        }
        _a_baro = (float) (getX(2));
        _v_baro = (float) (getX(1));
        return true;
    }

    double altitudeVariance(void) {
        return _altVar;
    }
    double verticalSpeedVariance(void) {
        return _varioVar;
    }
    double verticalSpeed(void) {
        return _v_baro;
    }
    double verticalAcceleration(void) {
        return _a_baro;
    }
    float timeOfLastStep(void) {
        return _time;
    }
    float pressureOfLastStep(void) {
        return _pressure;
    }
  protected:

    double _altVar = 0.01;   // Variance of altitude
    double _varioVar = 0.01; // Variance of vertical_speed
    float _v_baro = 0.0;     // EKF estimate: vertical_speed
    float _a_baro = 0.0;     // EKF estimate: vertical_acceleration
    float _time;             // tracked but unused; as passed via init_Kalman() and accKalman()
    float _pressure;         // tracked but unused; as passed via init_Kalman() and accKalman()
    uint32_t _step_errors;

    RunningStats _altitude_variance;

    void setR_M(int i, int j, double value) {
        this->setR(i, j, value);
    }

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


#endif
