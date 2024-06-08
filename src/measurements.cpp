#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "meteo.hpp"
#ifdef EKF
    #include "hKalF_acc.h"
#endif
#include "TimerStats.hpp"
#include "ArduinoJson.h"
#include "broker.hpp"
#include "pindefs.h"
#ifdef DEM_SUPPORT
    #include "demlookup.hpp"
    #include "protomap.hpp"
#endif
#include <ctime>

TimerStats kalman_step;

void kalman_stepz_error(double x0, double x1, double x2)  {
    log_e("step error  x0=%f x1=%f x2=%f", x0, x1, x2);
}

void process_pressure(const baroSample_t s) {
    if (s.type != SAMPLE_PRESSURE) {
        return; // TBD: use temperature
    }

    dps_sensors_t *dev = s.dev;      // device the sample came from

    float alt = altitude_from_pressure(s.value);
    float timestamp_sec = s.timestamp; // sensor time of sample

    // initialize first time around
    if (dev->previous_time < 0.0) {
        dev->previous_alt = alt;
        dev->previous_time = timestamp_sec;
        return;
    }
    if (dev->previous_alt < 0) {
        dev->previous_alt = alt;
        return;
    }
    // now dev->previous_alt and dev->previous_time are valid

    // compute vertical speed
    float delta_alt = alt - dev->previous_alt;           // meter
    float delta_t = timestamp_sec - dev->previous_time;  // sec
    float vertical_speed = delta_alt/delta_t;       // ms/s


#ifdef EKF
    // prime the altitude variance
    if (dev->initial_alt_values > 0) {
        log_d("%s: prime(alt=%.2f, vspeed=%.2f)", dev->dev.topic, alt, vertical_speed);
        dev->ekf->prime(alt, vertical_speed);
        dev->initial_alt_values--;
        return;
    }

    // log_i("ts %f p %f alt %f vs %f",timestamp_sec, s.value, alt, vertical_speed);

    kalman_step.Start();
    bool success = dev->ekf->accKalman(timestamp_sec, s.value, alt, vertical_speed);
    kalman_step.Stop();
#else
    bool success = true;
#endif
    if (success) {

        JsonDocument json;
        json["time"] = timestamp_sec;
        json["hPa"] = s.value;
        json["altitude"] = alt;
        json["verticalSpeedRaw"] = vertical_speed;

#ifdef EKF
        json["verticalSpeedKF"] = dev->ekf->verticalSpeed();
        json["verticalAccelerationKF"] = dev->ekf->verticalAcceleration();
        json["altitudeVariance"] = dev->ekf->altitudeVariance();
        json["verticalSpeedVariance"] = dev->ekf->verticalSpeedVariance();
#endif
        auto publish = mqtt.begin_publish(dev->dev.topic, measureJson(json));
        serializeJson(json, publish);
        publish.send();
#if 0
        log_i("t=%.1f pressure=%.2f v_baro=%.2f a_baro=%.2f altVar=%.3f varioVar=%.3f kalman_step=%d uS",
              dev->ekf->timeOfLastStep(),
              dev->ekf->pressureOfLastStep(),
              dev->ekf->verticalSpeed(),
              dev->ekf->verticalAcceleration(),
              dev->ekf->altitudeVariance(),
              dev->ekf->verticalSpeedVariance(),
              (int32_t) kalman_step.Mean());
#endif
    } else {
#ifdef EKF
        kalman_stepz_error(dev->ekf->getX(0), dev->ekf->getX(1), dev->ekf->getX(2));
#endif
    }
}


void process_imu( const imuSample_t &is) {

    if ((is.data.header & DMP_header_bitmap_Quat9) > 0) {
        double x = ((double)is.data.Quat9.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
        double y = ((double)is.data.Quat9.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
        double z = ((double)is.data.Quat9.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
        double w = sqrt(1.0 - ((x * x) + (y * y) + (z * z)));
        if (!isnan(w)) {
            JsonDocument json;
            json["time"] = is.timestamp *1.0e-6;
            json["ref"] = apply_reference_correction.get();
            if (set_reference_correction) {
                // store as conjugate of current quaternion
                ref_x = -x;
                ref_y = -y;
                ref_z = -z;
                ref_w = w;
                log_d("refpos %f %f %f %f", ref_x, ref_y, ref_z, ref_w);
                set_reference_correction = false;
            }
            // apply mounting position correction
            // https://invensense.tdk.com/wp-content/uploads/2024/03/eMD_Software_Guide_ICM20948.pdf
            // page 10
            if (apply_reference_correction) {
                double out_w = ref_w*w - ref_x*x - ref_y*y - ref_z*z;
                double out_x = ref_w*x + ref_x*w + ref_y*z - ref_z*y;
                double out_y = ref_w*y + ref_y*w + ref_z*x - ref_x*z;
                double out_z = ref_w*z + ref_z*w + ref_x*y - ref_y*x;
                // Normalize
                float tmp = sqrtf(out_w*out_w + out_x*out_x + out_y*out_y + out_z*out_z);
                if (tmp > 0 ) {
                    out_w /= tmp;
                    out_x /= tmp;
                    out_y /= tmp;
                    out_z /= tmp;
                }
                json["w"] = out_w;
                json["x"] = out_x;
                json["y"] = out_y;
                json["z"] = out_z;
            } else {
                json["w"] = w;
                json["x"] = x;
                json["y"] = y;
                json["z"] = z;
            }
            double hdg = atan2(2*x*y + 2*z*w, 1 - 2*y*y - 2*z*z)*(180.0/PI);
            hdg += heading_correction;
            if(hdg < 0) hdg = 360 + hdg;
            float deg = lround((360 - hdg)*10.0)/10.0;
            json["hdg"] = deg;
            json["heading_correction"] = heading_correction.get();

            auto hdg_publish = mqtt.begin_publish("imu/orientation", measureJson(json));
            serializeJson(json, hdg_publish);
            hdg_publish.send();
        }
    }
}

void process_gps( const gpsSample_t &gs) {
    const UBX_NAV_PVT_data_t &ub_nav_pvt = gs.nav_data;
    std::tm timeinfo = {};
#ifdef DEM_SUPPORT
    locInfo_t li = {};
    double ele;
    int rc = LS_INVALID;
#endif
    JsonDocument json;
    json["time"] = micros() * 1.0e-6;
    json["fixType"] = ub_nav_pvt.fixType;

    switch (ub_nav_pvt.fixType) {
        case 4:
        case 3:
#ifdef STATS
            gps_stats.Push(ub_nav_pvt.hMSL / 1000.0);
#endif
            json["hMSL"] = ub_nav_pvt.hMSL / 1000.0;
            json["hAE"] = ub_nav_pvt.height / 1000.0;
            json["velD"] = ub_nav_pvt.height / 1000.0;
            json["gSpeed"] = ub_nav_pvt.gSpeed / 1000.0;
            json["headMot"] = ub_nav_pvt.headMot * 1e-5;
            json["hAcc"] = ub_nav_pvt.hAcc * 0.01;
            json["vAcc"] = ub_nav_pvt.vAcc * 0.01;
            json["pDOP"] = ub_nav_pvt.pDOP * 0.01;
            __attribute__ ((fallthrough));
        case 2: {
                double lat = ub_nav_pvt.lat * 1e-7;
                double lon = ub_nav_pvt.lon * 1e-7;
                json["lat"] = lat;
                json["lon"] = lon;
#ifdef DEM_SUPPORT
                rc = getLocInfo(lat, lon, &li);
#endif
            }
            __attribute__ ((fallthrough));
        default:
            json["numSV"] = ub_nav_pvt.numSV;
            if (ub_nav_pvt.valid.bits.validDate) {
                json["year"] = ub_nav_pvt.year;
                json["month"] = ub_nav_pvt.month;
                json["day"] = ub_nav_pvt.day;
                timeinfo.tm_year = ub_nav_pvt.year - 1900;
                timeinfo.tm_mon = ub_nav_pvt.month - 1;
                timeinfo.tm_mday = ub_nav_pvt.day;
            }
            if (ub_nav_pvt.valid.bits.validTime) {
                json["hour"] = ub_nav_pvt.hour;
                json["min"] = ub_nav_pvt.min;
                json["sec"] = ub_nav_pvt.sec;
                timeinfo.tm_hour =  ub_nav_pvt.hour;
                timeinfo.tm_min = ub_nav_pvt.min;
                timeinfo.tm_sec = ub_nav_pvt.sec;
                timeinfo.tm_isdst = -1;
                if (ub_nav_pvt.valid.bits.validDate) {
                    std::time_t epoch_time = std::mktime(&timeinfo);
                    json["epoch"] = epoch_time; // Unix epoch, sec since 1-1-1970 UTC
                }
            }
            if (ub_nav_pvt.valid.bits.validMag) {
                json["magDec"] = ub_nav_pvt.magDec;
            }
    }
    {
        // see https://github.com/mlesniew/PicoMQTT/issues/38
        auto publish = mqtt.begin_publish("gps/nav", measureJson(json));
        serializeJson(json, publish);
        publish.send();
    }
#ifdef DEM_SUPPORT
    if (li.status == LS_VALID) {
        json.clear();
        json["time"] = micros() * 1.0e-6;
        json["meters"] = li.elevation;
        json["lat"] = ub_nav_pvt.lat * 1e-7;
        json["lon"] = ub_nav_pvt.lon * 1e-7;

        auto dempublish = mqtt.begin_publish("dem/elevation", measureJson(json));
        serializeJson(json, dempublish);
        dempublish.send();
    }
#endif
}


void process_measurements(void) {
    // TOGGLE(TRIGGER4);

    if (irq_queue_full || measurements_queue_full || commit_fail) {
        log_e("irq_queue_full=%d measurements_queue_full=%d commit_fail=%d",
              irq_queue_full, measurements_queue_full, commit_fail);
        irq_queue_full = 0;
        measurements_queue_full = 0;
        commit_fail = 0;
        log_i("rbfree %u maxitem %u", measurements_queue->curr_free_size(), measurements_queue->max_item_size());

    }

    size_t sz;
    void *p;

    if  ((p = measurements_queue->receive(&sz, 0)) != nullptr) {
        baroSample_t *bs = static_cast<baroSample_t *>(p);
        i2c_gendev_t *gd = &bs->dev->dev;

        switch (gd->type) {
            case DEV_DPS368: {
                    baroSample_t sample = * static_cast<baroSample_t *>(p);
                    measurements_queue->return_item(p);
                    process_pressure(sample);
                }
                break;
            case DEV_ICM_20948: {
                    imuSample_t sample = * static_cast<imuSample_t *>(p);
                    measurements_queue->return_item(p);
                    process_imu(sample);
                }
                break;
            case DEV_NEO_M9N: {
                    gpsSample_t sample = * static_cast<gpsSample_t *>(p);
                    measurements_queue->return_item(p);
                    process_gps(sample);
                }
                break;
            // case DEV_MFRC522: { // FIXME

            //     }
            // break;
            default:
                log_e("invalid sample type %d size %u", gd->type, sz);
                measurements_queue->return_item(p);
        }
        // yield();
    }
    // TOGGLE(TRIGGER4);

}