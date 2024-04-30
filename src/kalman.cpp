#include <Arduino.h>

#include "ekf_params.hpp"
#include "hKalF_acc.h"

static HKalF ekf;

// unshared
static float  finfo_FTime[11];    // time axis, seconds
static float  finfo_FBaro[11];    // altitude, meters
static float  finfo_FVario[11];   // vertical_speed, m/sec
static float finfo_vSpeed[11];    // EKF estimate: vertical_speed
static float finfo_a_vSpeed[11];  // EKF estimate: vertical_acceleration

double altVar = 0.01;    // Variance of altitude
double varioVar = 0.01;  // Variance of vertical_speed
float v_baro = 0.0;      // EKF estimate: vertical_speed
float a_baro = 0.0;      // EKF estimate: vertical_acceleration

void accKalman(unsigned long t, float pressure, float altitude, float vertical_speed) {

    for (int i = 0; i<10; ++i) {
        finfo_FTime[i] = finfo_FTime[i+1];
        finfo_FBaro[i] = finfo_FBaro[i+1];
        finfo_FVario[i] = finfo_FVario[i+1];
    }

    finfo_FTime[10] = t/1000.0; // seconds
    finfo_FBaro[10] = altitude;
    finfo_FVario[10] = vertical_speed;

    altVar = infoStatistics(finfo_FTime, finfo_FBaro);
    // sigmaKal Faktor = 3.0 aus st_hlballon.cpp

    altVar =  3.0 * altVar;
    ekf.setR_M(0, 0, altVar);
    double z[1] = {altitude};

    // Indication Kalman Error - Haik
    bool ok = true;
    if (!ekf.step(z)) {
        kalman_stepz_error(ekf.getX(0), ekf.getX(1), ekf.getX(2));
        ok = false;

        // sure it makes senser to continue afterall? FIXME
    }
    a_baro = (float) (ekf.getX(2));
    v_baro = (float) (ekf.getX(1));

    kalman_report(ok, pressure, v_baro, a_baro, altVar, varioVar);

    for (int i = 0; i<10; ++i) {
        finfo_vSpeed[i] = finfo_vSpeed[i+1];
        finfo_a_vSpeed[i] = finfo_a_vSpeed[i+1];
    }
    finfo_vSpeed[10] = v_baro;
    finfo_a_vSpeed[10] = a_baro;
}



//***************** Kalman Statistics Test *****************

float infoStatistics(float f_Time[11], float f_Elv[11]) {
    float sum = 0.0, mean, variance = 0.0;
    int i;
    for(i = 6; i < 11; ++i)
        sum += f_Elv[i];
    mean = sum/5;
    for(i = 6; i < 11; ++i)
        variance += pow(f_Elv[i] - mean, 2);
    variance=variance/5;
    return sqrt(variance);
}

void init_Kalman(uint32_t now, float initial_altitude) {
    for (int i = 0; i<11; ++i) {
        finfo_FTime[i] = now/1000.0;
        finfo_FBaro[i] = initial_altitude;
        finfo_FVario[i] = 0.0;
        finfo_vSpeed[i] = 0.0;
        finfo_a_vSpeed[i] = 0.0;
    }
}