#include "baro.hpp"
#include "params.hpp"

dps_sensors_t dps_sensors[] = {
    {
        .i2caddr = 0x76,
        .irq_pin = DPS0_IRQ_PIN,
        .status = 0,
        .initialized = false,
        .sensor = nullptr,
        .temp_measure_mask = 7,
        .wire = &Wire,
        .temp_osr = TEMP_OVERSAMPLING_RATE,
        .prs_osr = PRESS_OVERSAMPLING_RATE,
        .previous_alt = -1e6,
        .previous_time = -1e6,
        .initial_alt_values = INITIAL_ALTITUDE_VALUES,
        .topic = "dps368-0"
    }
};
uint32_t num_sensors = (sizeof(dps_sensors)/sizeof(dps_sensors[0]));

