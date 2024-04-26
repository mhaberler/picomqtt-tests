#include <M5Unified.h>
#include <Wire.h>
#include <PicoMQTT.h>
#include <PicoWebsocket.h>
#include <ESPmDNS.h>

#if __has_include("myconfig.h")
    #include "myconfig.h"
#endif
#define INTERVAL 3000

void webserver_setup(void);
void webserver_loop(void);

::WiFiServer server(81);
PicoWebsocket::Server<::WiFiServer> websocket_server(server);
PicoMQTT::Server mqtt(websocket_server);

void setup() {

    delay(3000);
    M5.begin();
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    webserver_setup();

    mqtt.begin();

    mqtt.subscribe("#", [](const char * topic, const char * payload) {
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });


}
uint32_t last;

void loop() {
    mqtt.loop();
    webserver_loop();
    // periodically publish CPU temperature
    if (millis() - last > INTERVAL) {
        float t = temperatureRead();
        String topic = "picomqtt/esp-cpu-temperature";
        String message = String(t);
        mqtt.publish(topic, message);
        mqtt.loop();

        topic = "esp32/free-heap";
        message = String(esp_get_free_heap_size());
        mqtt.publish(topic, message);

        last = millis();
    }

    yield();
}
