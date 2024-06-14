#include "sensor.hpp"
#include "params.hpp"
#include "settings.hpp"
#include "pindefs.h"
#include "broker.hpp"
#include "tickers.hpp"

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
#ifdef EKF
        .ekf = new HKalF(),
#endif
#ifdef SMOOTH
        .alt = new ExponentialSmoothing(),
        .vspeed = new ExponentialSmoothing()
#endif
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
#ifdef EKF
        .ekf = new  HKalF(),
#endif
#ifdef SMOOTH
        .alt = new ExponentialSmoothing(),
        .vspeed = new ExponentialSmoothing()
#endif
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
#ifdef EKF
        .ekf = new HKalF(),
#endif
#ifdef SMOOTH
        .alt = new ExponentialSmoothing(),
        .vspeed = new ExponentialSmoothing()
#endif
    }
#endif
};
uint32_t num_dps_sensors = (sizeof(dps_sensors)/sizeof(dps_sensors[0]));

#if defined(IMU_SUPPORT)

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
#endif

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


PicoSettings baro_settings(mqtt, "baro");

// lost interrupts:
// if we have not heard from a DPS3xx in i2c_timeout seconds, re-init device
float_setting_t i2c_timeout(baro_settings, "i2c_timeout", 10.0);

// exponential smoothing factor for baro samples
int_setting_t alpha_pct(baro_settings, "alpha", 90);

// icm20948.cpp
PicoSettings imu_settings(mqtt, "imu");

bool_setting_t set_reference_correction(imu_settings, "refpos", false);
bool_setting_t apply_reference_correction(imu_settings, "apply_refpos", true);

// default to identity quaternion
double_setting_t ref_x(imu_settings, "ref_x", 0.0);
double_setting_t ref_y(imu_settings, "ref_y", 0.0);
double_setting_t ref_z(imu_settings, "ref_z", 0.0);
double_setting_t ref_w(imu_settings, "ref_w", 1.0);

double_setting_t heading_correction(imu_settings, "hdg_corr", 0.0);
float_setting_t quat9_rate(imu_settings, "quat9_rate", 5.0);

// ublox.cpp
PicoSettings gps_settings(mqtt, "gps");

void set_nav_rate(int nav_rate);
int_setting_t nav_rate(gps_settings, "nav_rate", 1, [] {
    set_nav_rate(nav_rate.get());
});

void set_alpha(int x) {
    double a = alpha_pct.get() / 100.0;
    for (auto s : dps_sensors) {
        s.vspeed->setAlpha(a);
        s.alt->setAlpha(a);
    }
}

void settings_tick() {
    imu_settings.publish();
    baro_settings.publish();
    gps_settings.publish();
}

void alpha_pct_cb(void) {
    set_alpha(alpha_pct.get());
}

void  settings_setup() {
    imu_settings.begin();
    baro_settings.begin();
    gps_settings.begin();

    set_alpha(alpha_pct.get()); // watch init order
    alpha_pct.change_callback = alpha_pct_cb;
}
