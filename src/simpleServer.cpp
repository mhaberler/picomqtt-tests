#include <esp-fs-webserver.h>  // https://github.com/cotestatnt/esp-fs-webserver

#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS
#if ESP_FS_WS_USE_SD
    #include <SD.h>
#endif
#include "pindefs.h"
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

uint32_t cardSize, mbfree;
const char *sd_type ="none";
#if ESP_FS_WS_USE_SD

sdcard_type_t sdType;

bool mountSDCard(void) {
    if  (!SD.begin(SPI0_SLAVE_SELECT_SDCARD, SPI, SD_SPEED, "/sd")) {
        log_error("Card Mount Failed");
        return false;
    }

    switch (SD.cardType()) {
        case CARD_NONE:
            sd_type = "No SD card present";
            cardSize = 0;
            mbfree = 0;
            return false;
        case CARD_MMC:
            sd_type = "MMC";
            break;
        case CARD_SD:
            sd_type = "SDSC";
            break;
        case CARD_SDHC:
            sd_type = "SDHC";
            break;
        default:
            sd_type = "unknown";
            break;
    }
    cardSize = SD.cardSize() / (1024 * 1024);
    mbfree = (SD.totalBytes() - SD.usedBytes())/ (1024 * 1024);
    log_info("SD Card: type %s Size: %u MB free %u MB", sd_type, cardSize, mbfree);
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
    myWebServer.printFileList(LittleFS, Serial, "/", 1);
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
        myWebServer.printFileList(SD, Serial, "/", 1);
    }
#endif
    RGBLED(0,64,0);

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

    RGBLED(0,0,64);

}


void webserver_loop() {
    myWebServer.run();
}