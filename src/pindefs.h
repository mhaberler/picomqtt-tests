

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
    #define IMU_IRQ_PIN 10 // yellow 4

    // DPS368
    #define DPS0_IRQ_PIN 6  // blue 6
    #define DPS1_IRQ_PIN 7  // violet 7
    #define DPS2_IRQ_PIN 5

    // SD card
    #define SD_INSERTED -1
    #define SPI0_SLAVE_SELECT_SDCARD 4
    #define SD_SPEED 25000000
    #define SPI0_CLOCK -1
    #define SPI0_MOSI -1
    #define SPI0_MISO -1

    #define TRIGGER1 18 // blue 14
    #define TRIGGER2 17 // violett 15
    #define TRIGGER3 13 // yellow 12
    #define TRIGGER4 0 // green 13

    #ifdef QUADRATURE_DECODER
        #define PIN_FLOWSENSOR_A 17  // blue port
        #define PIN_FLOWSENSOR_B 18
    #endif


#endif


#if defined(M5STAMP_C3U)
    #define I2C0_SDA 1
    #define I2C0_SCL 0
    #define I2C0_SPEED 400000

    // #define I2C1_SDA 19
    // #define I2C1_SCL 18
    // #define I2C1_SPEED 400000

    #define NEOPIXEL_LED 2


    // GPS - I2C1
    // #define UBLOX_PPS_PIN 4

    // IMU - I2C0
    //  top left edge downwards

    #define IMU_IRQ_PIN 3 //aussen

    // DPS368
    #define DPS0_IRQ_PIN 4


    // // SPI - SD Card
    // #define SPI0_CLOCK 14
    // #define SPI0_MOSI 12
    // #define SPI0_MISO 13
    // #define SPI0_SLAVE_SELECT_SDCARD 11

    // SD card
    #define SD_INSERTED -1
    #define SD_SPEED 40000000

    #define TRIGGER1 5
    #define TRIGGER2 6

    #ifdef FLOWSENSOR
        #define PIN_FLOWSENSOR_A 13  // blue port
    #endif

    #ifdef QUADRATURE_DECODER
        #define PIN_FLOWSENSOR_A 8 
        #define PIN_FLOWSENSOR_B 10
    #endif


#endif


#if defined(ARDUINO_M5STACK_Core2) && defined (BOTTOM2)
    // IMU - I2C0
    #define IMU_IRQ_PIN 27

    // DPS368
    #define DPS0_IRQ_PIN 19
    #define DPS0_WIRE Wire1

    // #define DPS1_IRQ_PIN 7
    // #define DPS2_IRQ_PIN 5

    // SD card
    #define SD_INSERTED -1
    #define SPI0_SLAVE_SELECT_SDCARD 4
    #define SD_SPEED 25000000
    #define SPI0_CLOCK -1
    #define SPI0_MOSI -1
    #define SPI0_MISO -1

    #ifdef FLOWSENSOR
        #define PIN_FLOWSENSOR_A 13  // blue port
    #endif

    #ifdef QUADRATURE_DECODER
        #define PIN_FLOWSENSOR_A 13  // blue port
        #define PIN_FLOWSENSOR_B 14
    #endif

#endif

#if defined(ARDUINO_M5Stack_StampS3)
    #define I2C0_SDA 13 // grove
    #define I2C0_SCL 15
    #define I2C0_SPEED 400000

    #define I2C1_SDA 1    // grove 2 white
    #define I2C1_SCL 3    // grove 1 yellow
    #define I2C1_SPEED 400000

    #define IMU_IRQ_PIN 5 //aussen

    // DPS368
    #define DPS0_IRQ_PIN 7

    #define NEOPIXEL_LED 21

    #define SD_INSERTED -1

    #ifdef FLOWSENSOR
        #define PIN_FLOWSENSOR_A 5
    #endif

    #ifdef QUADRATURE_DECODER
        #define PIN_FLOWSENSOR_A 5
        #define PIN_FLOWSENSOR_B 7
    #endif

#endif

#if defined(TRACE_PINS)
#define TRIGGER_SETUP(pin) \
    pinMode(pin, OUTPUT);\
    digitalWrite(pin, 0);
#define TOGGLE(p)     digitalWrite(p, ! digitalRead(p))
#else
#define TRIGGER_SETUP(pin)
#define TOGGLE(p)
#endif

#ifdef NEOPIXEL_LED
    #define RGBLED(red, green, blue) neopixelWrite(NEOPIXEL_LED, red, green, blue)
#else
    #define RGBLED(red, green, blue)
#endif