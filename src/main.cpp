#include <M5Unified.h>
#include <Wire.h>
#include <PicoMQTT.h>
#include <PicoWebsocket.h>
#include <ESPmDNS.h>
#include "tickers.hpp"
#include "i2cio.hpp"
#include "demlookup.hpp"

#if __has_include("myconfig.h")
    #include "myconfig.h"
#endif


void webserver_setup(void);
void webserver_loop(void);
void sensor_setup(void);
void sensor_loop(void);
void initSDCard(void);

::WiFiServer mqtt_tcp_server(MQTT_TCP);
::WiFiServer mqtt_ws_server(MQTT_WS);
PicoWebsocket::Server<::WiFiServer> websocket_server(mqtt_ws_server);
PicoMQTT::Server mqtt(mqtt_tcp_server, websocket_server);

TICKER(internal, INTERVAL);

void setup() {

    delay(3000);
    M5.begin();

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    Wire.begin();
    i2c_scan(Wire);

    log_i("mounting SD card");
    initSDCard();
#ifdef DEM_SUPPORT
    dem_setup();
#endif
    webserver_setup();
    sensor_setup();

    mqtt.begin();
    mqtt.subscribe("esp32/interval", [](const char * topic, const char * payload) {
        uint32_t new_interval = strtoul (payload, NULL, 0);
        if (new_interval  > MIN_INTERVAL) {
            CHANGE_TICKER(internal, new_interval);
            Serial.printf("changed ticker to %u ms\n", new_interval);
        }
    });
    RUNTICKER(internal);
}


void loop() {
    mqtt.loop();
    webserver_loop();
    sensor_loop();

    if (TIME_FOR(internal)) {
        float t = temperatureRead();
        String topic = "esp32/esp-cpu-temperature";
        String message = String(t);
        mqtt.publish(topic, message);
        mqtt.publish("esp32/interval", String(internal_update_ms));
        mqtt.loop();

        topic = "esp32/free-heap";
        message = String(ESP.getFreeHeap());
        mqtt.publish(topic, message);

        DONE_WITH(internal);
    }
    yield();
}
