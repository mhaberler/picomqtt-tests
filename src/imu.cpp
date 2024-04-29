#include <Arduino.h>
#include "i2cio.hpp"
#include <ICM_20948.h>
#include <ArduinoJson.h>
#include "timingpin.hpp"
#include "tickers.hpp"
#include "imu.hpp"
#include "broker.hpp"

static ICM_20948_I2C icm;

bool imu_setup( void) {
    if (detect(Wire, ICM_20948_I2C_ADDR_AD1)) {
        bool success = true;

        icm.begin(Wire, ICM_20948_I2C_ADDR_AD1);
        log_i("status: %s", icm.statusString());

        success &= (icm.swReset() == ICM_20948_Stat_Ok);
        delay(250);
        success &= (icm.sleep(false) == ICM_20948_Stat_Ok);
        success &= (icm.lowPower(false) == ICM_20948_Stat_Ok);

        success &= (icm.initializeDMP() == ICM_20948_Stat_Ok);
        success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok); // 9-axis quaternion + heading accuracy
        success &= (icm.enableDMPSensor(INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD) == ICM_20948_Stat_Ok); // 32-bit calibrated compass
        success &= (icm.setDMPODRrate(DMP_ODR_Reg_Quat9, 10) == ICM_20948_Stat_Ok); // Set to 5Hz for motion quaternion

        success &= (icm.setDMPODRrate(DMP_ODR_Reg_Cpass_Calibr, 10) == ICM_20948_Stat_Ok); // Set to 5Hz

        success &= (icm.enableFIFO() == ICM_20948_Stat_Ok);
        success &= (icm.enableDMP() == ICM_20948_Stat_Ok);
        success &= (icm.resetDMP() == ICM_20948_Stat_Ok);
        success &= (icm.resetFIFO() == ICM_20948_Stat_Ok);

        log_i("status: %s %d success=%d", icm.statusString(), icm.status, success);

        return success;
        return (icm.status == ICM_20948_Stat_Ok) || (icm.status ==  ICM_20948_Stat_NoData);
    }
    return false;
}

void imu_loop( void) {
    icm_20948_DMP_data_t data;
    icm.readDMPdataFromFIFO(&data);
    unsigned long now = micros();

    if ((icm.status == ICM_20948_Stat_Ok) || (icm.status == ICM_20948_Stat_FIFOMoreDataAvail)) {

        if ((data.header & DMP_header_bitmap_Quat9) > 0) {
            double q1 = ((double)data.Quat9.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
            double q2 = ((double)data.Quat9.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
            double q3 = ((double)data.Quat9.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
            double q0 = sqrt(abs(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3))));

            JsonDocument json;

            json["us"] = now;

            json["w"] = q0;
            json["x"] = q1;
            json["y"] = q2;
            json["z"] = q3;

            auto publish = mqtt.begin_publish("quat", measureJson(json));
            serializeJson(json, publish);
            publish.send();
        }
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
    }
}