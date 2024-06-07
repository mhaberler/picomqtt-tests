#pragma once

#include <Dps3xx.h>
#include <ICM_20948.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522Extended.h>
#include <NfcAdapter.h>

#include "irq.hpp"
#ifdef EKF
    #include "hKalF_acc.h"
#endif

typedef enum {
    DEV_NONE=0,
    DEV_DPS368,
    DEV_ICM_20948,
    DEV_M5STACK_IMU,
    DEV_NEO_M9N,
    DEV_MFRC522,
    DEV_BATTERY,
    DEV_MICROPHONE,
} device_t;

typedef enum {
    SAMPLE_INVALID,
    SAMPLE_PRESSURE,
    SAMPLE_TEMPERATURE,
    SAMPLE_ORIENTATION,
    SAMPLE_GPS,
} sample_t;

// extend status code from dps_config.h
// #define DPS__SUCCEEDED 0
// #define DPS__FAIL_UNKNOWN -1
// #define DPS__FAIL_INIT_FAILED -2
// #define DPS__FAIL_TOOBUSY -3
// #define DPS__FAIL_UNFINISHED -4

#define DPS__STANDBY_FAILED      -100
#define DPS__MEAS_TEMP_FAILED    -101
#define DPS__STANDBY2_FAILED     -102
#define DPS__SETIRQSRC_FAILED    -103
#define DPS__INTSTAT_RDY_FAILED  -104
#define DPS__MEAS_PRS_FAILED     -105
#define DPS__DEVICE_NOT_RESPONDING     -106


typedef struct  {
    device_t type;
    uint8_t i2caddr;
    uint8_t irq_pin;
    uint8_t irq_pin_mode;
    uint8_t irq_pin_edge;
    bool irq_attached;
    bool device_present;
    bool device_initialized;
    uint32_t init_count;
    uint32_t softirq_count;
    float last_heard; // fseconds()
    // MQTT-related
    const char *topic;
} i2c_gendev_t;

typedef struct  {
    i2c_gendev_t dev;
    uint8_t irq_polarity;
    uint8_t status;
    Dps3xx *sensor;
    uint32_t temp_measure_mask;
    TwoWire *wire;
    int16_t temp_osr;
    int16_t prs_osr;
    // per-instance tracking values
    float previous_alt;
    float previous_time;
    uint32_t initial_alt_values;
#ifdef EKF
    HKalF *ekf; // per-device EKF instance
#endif
} dps_sensors_t;

// Setting value can be calculated as follows:
// Value = (DMP running rate (225Hz) / ODR ) - 1
// E.g. For a 25Hz ODR rate, value= (225/25) - 1 = 8.

#define DMP_RATE_CONF(hz) ((int)((225.0 / hz ) - 1.0))

typedef struct  {
    i2c_gendev_t dev;
} devhdr_t;

typedef struct  {
    i2c_gendev_t dev;
    TwoWire *wire;
    ICM_20948_I2C *icm;
} icm20948_t;

typedef struct  {
    i2c_gendev_t dev;
    TwoWire *wire;
    SFE_UBLOX_GNSS *neo;
    uint8_t navFreq;
    bool trace;
} gps_sensor_t;

typedef struct  {
    i2c_gendev_t dev;
    TwoWire *wire;
    MFRC522DriverI2C *driver;
    MFRC522Extended *mfrc522;
    NfcAdapter *nfc;
    MFRC522::MIFARE_Key *key;
    bool driver_instantiated;
} nfc_reader_t;

typedef struct  {
    i2c_gendev_t dev;
} battery_status_t;

typedef struct  {
    dps_sensors_t *dev;
    sample_t type;
    float timestamp;   // sec
    float value;       // hPa/degC
} baroSample_t;

typedef struct  {
    icm20948_t  *dev;
    uint8_t dev_id;
    sample_t type;
    icm_20948_DMP_data_t data;
    float timestamp;   // uS
} imuSample_t;

typedef struct  {
    gps_sensor_t *dev;
    uint8_t dev_id;
    sample_t type;
    UBX_NAV_PVT_data_t nav_data;
    float timestamp;   // uS
} gpsSample_t;

int16_t dps368_setup(int i);
void process_measurements(void);
void process_pressure(const baroSample_t s);
void process_imu( const imuSample_t &is);

void post_softirq(void *dev);

extern dps_sensors_t dps_sensors[];
extern uint32_t num_dps_sensors;
extern icm20948_t imu_sensor;
extern gps_sensor_t gps_sensor;

void sensor_loop(void);
void sensor_setup(void);

void ublox_loop(void);
bool ublox_setup(void);

void set_hdg_corr(double degrees);
bool imu_setup(icm20948_t *imu_sensor );
void imu_loop(void);
