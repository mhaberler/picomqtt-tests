#include <esp-fs-webserver.h>  // https://github.com/cotestatnt/esp-fs-webserver

#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS
#if ESP_FS_WS_USE_SD
    #include <SD.h>
    #include "pindefs.h"
#endif
#define DBG_OUTPUT_PORT     Serial
#define LOG_LEVEL           2         // (0 disable, 1 error, 2 info, 3 debug)
#include "SerialLog.h"

FSWebServer myWebServer(FILESYSTEM, HTTP_PORT, HOSTNAME);

// In order to set SSID and password open the /setup webserver page
// const char* ssid;
// const char* password;
bool apMode = false;

// an SD card was found and is mounted
bool sdcard_mounted = false;
// state of the card detect pin - assume inserted if no CD pin
bool sdcard_inserted = (SD_INSERTED > -1) ? false : true;



#if ESP_FS_WS_USE_SD
bool mountSDCard(void) {
    if  (!SD.begin(SPI0_SLAVE_SELECT_SDCARD, SPI, SD_SPEED, "/sd")) {
        log_error("Card Mount Failed");
        return false;
    }

    switch (SD.cardType()) {
        case CARD_NONE:
            log_info("No SD card attached");
            return false;
        case CARD_MMC:
            log_info("card type: MMC");
            break;
        case CARD_SD:
            log_info("card type: SDSC");
            break;
        case CARD_SDHC:
            log_info("card type: SDHC");
            break;
        default:
            log_info("card type: UNKNOWN");
            break;
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    uint64_t mbfree = (SD.totalBytes() - SD.usedBytes())/ (1024 * 1024);
    log_info("SD Card Size: %u MB free %u MB\n", (uint32_t) cardSize, (uint32_t) mbfree);
    return true;
}
#endif

////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem() {
    // FILESYSTEM INIT - LittleFS
    if ( !FILESYSTEM.begin()) {
        log_error("ERROR on mounting filesystem. It will be formmatted!");
        FILESYSTEM.format();
        ESP.restart();
    }
    myWebServer.printFileList(LittleFS, Serial, "/", 2);
}

/*
* Getting FS info (total and free bytes) is strictly related to
* filesystem library used (LittleFS, FFat, SPIFFS etc etc) and ESP framework
* ESP8266 FS implementation has methods for total and used bytes (only label is missing)
*/
#ifdef ESP32
void getFsInfo(fsInfo_t* fsInfo) {
    fsInfo->fsName = "LittleFS";
    fsInfo->totalBytes = LittleFS.totalBytes();
    fsInfo->usedBytes = LittleFS.usedBytes();
}
#else
void getFsInfo(fsInfo_t* fsInfo) {
    fsInfo->fsName = "LittleFS";
}
#endif

// static int ledval;

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
// void handleLed() {
//     // http://xxx.xxx.xxx.xxx/led?val=1
//     if (myWebServer.hasArg("val")) {
//         int value = myWebServer.arg("val").toInt();
// #ifdef RGB_LED
//         if (value) {
//             // digitalWrite(PIN_NEOPIXEL, HIGH);
//             neopixelWrite(PIN_NEOPIXEL,RGB_BRIGHTNESS,0,0); // Red
//         } else {
//             // digitalWrite(PIN_NEOPIXEL, LOW);    // Turn the RGB LED off

//             neopixelWrite(PIN_NEOPIXEL,0,0,0); // Off / black
//         }
//         ledval = value;
// #else
//         // digitalWrite(ledPin, value);
// #endif
//     }

//     String reply = "LED is now ";
// #ifdef RGB_BUILTIN
//     reply += ledval ? "OFF" : "ON";
// #else
//     // reply += digitalRead(ledPin) ? "OFF" : "ON";
// #endif
//     myWebServer.send(200, "text/plain", reply);
// }


void webserver_setup() {

   RGBLED(0,64,64);
#if ESP_FS_WS_USE_SD
    SPI.begin(SPI0_CLOCK, SPI0_MISO, SPI0_MOSI);
    if (SD_INSERTED > -1) {
        pinMode(SD_INSERTED, INPUT_PULLUP);
        sdcard_inserted = digitalRead(SD_INSERTED);
    }
    // no CD pin - mount right away
    sdcard_mounted = mountSDCard();
    if (sdcard_mounted) {
        myWebServer.printFileList(SD, Serial, "/", 2);
    }
#endif
    RGBLED(0,64,0);
    // FILESYSTEM INIT
    startFilesystem();

    // Try to connect to stored SSID, start AP if fails after timeout
    myWebServer.setAP(AP_SSID, AP_PASSWORD);

    IPAddress myIP = myWebServer.startWiFi(15000);

    // // Add custom page handlers to webserver
    // myWebServer.on("/led", HTTP_GET, handleLed);

    // set /setup and /edit page authentication
    // myWebServer.setAuthentication("admin", "admin");

    // Enable ACE FS file web editor and add FS info callback function
    myWebServer.enableFsCodeEditor(getFsInfo);

    // Start webserver
    myWebServer.begin();
    log_info("%s Web Server started on IP Address: %s",  HOSTNAME, myIP.toString().c_str());

    log_info("Open /setup page to configure optional parameters");
    log_info("Open /edit page to view and edit files");
    log_info("Open /restart page to reboot");



#ifdef RGB_LED
    neopixelWrite(PIN_NEOPIXEL, 0, RGB_BRIGHTNESS,0); // green
#endif
}


void webserver_loop() {
    myWebServer.run();
}