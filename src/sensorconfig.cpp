#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "pindefs.h"

dps_sensors_t dps_sensors[] = {
#if defined(DPS0_IRQ_PIN)
    {
        .dev = {
            .type = DEV_DPS368,
            .i2caddr = 0x77,
            .irq_pin = DPS0_IRQ_PIN,
            .irq_pin_mode = INPUT_PULLUP,
            .irq_pin_edge= FALLING,
            .irq_attached = false,
            .device_present = false,
            .device_initialized = false,
            .topic = "baro/0"
        },
        .irq_polarity = 0,
        .status = 0,
        .sensor = nullptr,
        .temp_measure_mask = 7,
#ifdef DPS0_WIRE
        .wire = &DPS0_WIRE,
#else
        .wire = &Wire,
#endif
        .temp_osr = TEMP_OVERSAMPLING_RATE,
        .prs_osr = PRESS_OVERSAMPLING_RATE,
        .previous_alt = -1e6,
        .previous_time = -1e6,
        .initial_alt_values = INITIAL_ALTITUDE_VALUES,
    },
#endif
#if defined(DPS1_IRQ_PIN)
    {
        .dev = {
            .type = DEV_DPS368,
            .i2caddr = 0x76,
            .irq_pin = DPS1_IRQ_PIN,
            .irq_pin_mode = INPUT,
            .irq_pin_edge= RISING,
            .irq_attached = false,
            .device_present = false,
            .device_initialized = false,
            .topic = "baro/1"
        },

        .irq_polarity = 1,
        .status = 0,
        .sensor = nullptr,
        .temp_measure_mask = 7,
        .wire = &Wire,
        .temp_osr = TEMP_OVERSAMPLING_RATE,
        .prs_osr = PRESS_OVERSAMPLING_RATE,
        .previous_alt = -1e6,
        .previous_time = -1e6,
        .initial_alt_values = INITIAL_ALTITUDE_VALUES,
    },
#endif
#if defined(DPS2_IRQ_PIN)
    {
        .dev = {
            .type = DEV_DPS368,
            .i2caddr = 0x77,
            .irq_pin = DPS2_IRQ_PIN,
            .irq_pin_mode = INPUT_PULLUP,
            .irq_pin_edge= FALLING,
            .irq_attached = false,
            .device_present = false,
            .device_initialized = false,
            .topic = "baro/2"
        },
        .irq_polarity = 0,
        .status = 0,
        .sensor = nullptr,
        .temp_measure_mask = 7,
        .wire = &Wire1,
        .temp_osr = TEMP_OVERSAMPLING_RATE,
        .prs_osr = PRESS_OVERSAMPLING_RATE,
        .previous_alt = -1e6,
        .previous_time = -1e6,
        .initial_alt_values = INITIAL_ALTITUDE_VALUES,
    }
#endif
};
uint32_t num_dps_sensors = (sizeof(dps_sensors)/sizeof(dps_sensors[0]));

extern ICM_20948_I2C icm;
icm20948_t imu_sensor = {
    .dev = {
        .type = DEV_ICM_20948,
        .i2caddr = ICM_20948_I2C_ADDR_AD1,
        .irq_pin = IMU_IRQ_PIN,
        .irq_pin_mode = INPUT, //  breakout board has pullup resistor
        .irq_pin_edge= FALLING,
        .irq_attached = false,
        .device_present = false,
        .device_initialized = false,
        .topic = "imu"
    },
    .wire = &Wire,
    .icm = &icm,
};

// ublox is polled I/O for now
extern SFE_UBLOX_GNSS ublox_neo;
gps_sensor_t gpsconf = {
    .dev = {
        .type = DEV_NEO_M9N,
        .i2caddr = UBLOX_I2C_ADDR,
        .irq_attached = false,
        .device_present = false,
        .device_initialized = false,
        .topic = "gps"
    },
    // Wire is autodetect, filled in during ublox_setup()
    .neo = &ublox_neo,
    .navFreq = NAV_FREQUENCY,
    .trace = false,
};

nfc_reader_t nfcconf = {
    .dev = {
        .type = DEV_MFRC522,
        .i2caddr = MRFC522_I2C_ADDR,
        .irq_attached = false,
        .device_present = false,
        .device_initialized = false,
        .topic = "nfc"
    },
    .wire = &NFC_WIRE,
    .driver_instantiated = false
};

battery_status_t battery_conf = {
    .dev = {
        .type = DEV_BATTERY,
        .topic = "battery"
    }
};