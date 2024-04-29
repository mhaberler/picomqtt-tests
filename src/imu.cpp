#include <Arduino.h>
#include "i2cio.hpp"
#include <ICM_20948.h>
#include <ArduinoJson.h>
#include <math.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "imu.hpp"
#include "broker.hpp"

#undef CALIBRATED_COMPASS // broken

static ICM_20948_I2C icm;

bool imu_setup( void) {
    if (detect(Wire, ICM_20948_I2C_ADDR_AD1)) {
        bool success = true;

        icm.begin(Wire, ICM_20948_I2C_ADDR_AD1);

        success &= (icm.swReset() == ICM_20948_Stat_Ok);
        delay(250);
        success &= (icm.sleep(false) == ICM_20948_Stat_Ok);
        success &= (icm.lowPower(false) == ICM_20948_Stat_Ok);

        success &= (icm.initializeDMP() == ICM_20948_Stat_Ok);
        success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok); // 9-axis quaternion + heading accuracy
        success &= (icm.setDMPODRrate(DMP_ODR_Reg_Quat9, 10) == ICM_20948_Stat_Ok); // Set to 5Hz for motion quaternion

#ifdef CALIBRATED_COMPASS
        success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD) == ICM_20948_Stat_Ok); // 32-bit calibrated compass
        success &= (icm.setDMPODRrate(DMP_ODR_Reg_Cpass_Calibr, 14) == ICM_20948_Stat_Ok);
#endif
        success &= (icm.enableFIFO() == ICM_20948_Stat_Ok);
        success &= (icm.enableDMP() == ICM_20948_Stat_Ok);
        success &= (icm.resetDMP() == ICM_20948_Stat_Ok);
        success &= (icm.resetFIFO() == ICM_20948_Stat_Ok);

        log_i("icm init: success=%d", success);
        return success;
    }
    return false;
}

void imu_loop( void) {
    icm_20948_DMP_data_t data;
    icm.readDMPdataFromFIFO(&data);
    unsigned long now = micros();

    if ((icm.status == ICM_20948_Stat_Ok) || (icm.status == ICM_20948_Stat_FIFOMoreDataAvail)) {

        if ((data.header & DMP_header_bitmap_Quat9) > 0) {
            double x = ((double)data.Quat9.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
            double y = ((double)data.Quat9.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
            double z = ((double)data.Quat9.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
            double w = sqrt(1.0 - ((x * x) + (y * y) + (z * z)));
            if (!isnan(w)) {
                double hdg = atan2(2*x*y + 2*z*w, 1 - 2*y*y - 2*z*z)*(180.0/PI);
                if(hdg < 0) hdg = 360 + hdg;
                hdg = round(360 - hdg);

                JsonDocument json;
                json["us"] = now;
                json["w"] = w;
                json["x"] = x;
                json["y"] = y;
                json["z"] = z;
                json["hdg"] = hdg;

                auto publish = mqtt.begin_publish("quat", measureJson(json));
                serializeJson(json, publish);
                publish.send();
            }
        }
#ifdef CALIBRATED_COMPASS
        if ((data.header & DMP_header_bitmap_Compass_Calibr) > 0)  {
            JsonDocument json;
            json["us"] = now;

            json["mx"] = data.Compass_Calibr.Data.X;
            json["my"] = data.Compass_Calibr.Data.Y;
            json["mz"] = data.Compass_Calibr.Data.Z;
            auto publish = mqtt.begin_publish("compass", measureJson(json));
            serializeJson(json, publish);
            publish.send();
        }
#endif
    }
}


