#if defined(DEVKITC)

#define I2C0_SDA 7
#define I2C0_SCL 8
#define I2C0_SPEED 400000

#define I2C1_SDA 5
#define I2C1_SCL 6
#define I2C1_SPEED 400000

#define NEOPIXEL_LED 38


// GPS - I2C1
// #define UBLOX_PPS_PIN 4

// IMU - I2C0
#define IMU_IRQ_PIN 1

// DPS368
#define DPS0_IRQ_PIN 47
#define DPS1_IRQ_PIN 48
#define DPS2_IRQ_PIN 4

// SPI - SD Card
#define SPI0_CLOCK 14
#define SPI0_MOSI 12
#define SPI0_MISO 13
#define SPI0_SLAVE_SELECT_SDCARD 11

// SD card
#define SD_INSERTED 10
#define SD_SPEED 40000000

// LA trigger pins
#define TRIGGER1 17
#define TRIGGER2 18

#define TOGGLE(p)     digitalWrite(p, ! digitalRead(p))


#endif

#if defined(ARDUINO_M5STACK_CORES3)

// IMU - I2C0
#define IMU_IRQ_PIN 10

// DPS368
#define DPS0_IRQ_PIN 6
#define DPS1_IRQ_PIN 7
#define DPS2_IRQ_PIN 5

// SD card
#define SD_INSERTED -1
#define SPI0_SLAVE_SELECT_SDCARD 4
#define SD_SPEED 25000000
#define SPI0_CLOCK -1
#define SPI0_MOSI -1
#define SPI0_MISO -1

#endif

#if defined(ARDUINO_M5STACK_Core2)
// IMU - I2C0
#define IMU_IRQ_PIN 10

// DPS368
#define DPS0_IRQ_PIN 3
// #define DPS1_IRQ_PIN 7
// #define DPS2_IRQ_PIN 5

// SD card
#define SD_INSERTED -1
#define SPI0_SLAVE_SELECT_SDCARD 4
#define SD_SPEED 25000000
#define SPI0_CLOCK -1
#define SPI0_MOSI -1
#define SPI0_MISO -1

#endif

#if defined(ARDUINO_ESP32C3_DEV)

#endif

#if defined(TRACE_PINS)
#define TOGGLE(p)     digitalWrite(p, ! digitalRead(p))
#else
#define TOGGLE(p) 
#endif

#ifdef NEOPIXEL_LED
#define RGBLED(red, green, blue) neopixelWrite(NEOPIXEL_LED, red, green, blue)
#else
#define RGBLED(red, green, blue)
#endif