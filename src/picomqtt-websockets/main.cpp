#include <Arduino.h>
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

MDNSResponder *mdns_responder;

::WiFiServer server(81);
PicoWebsocket::Server<::WiFiServer> websocket_server(server);
PicoMQTT::Server mqtt(websocket_server);

unsigned long last_publish_time = 0;

static const char flash_string[] PROGMEM = "This is a string stored in flash.";

void setup() {
    // Setup serial
    delay(3000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    webserver_setup();

    // // Connect to WiFi
    // Serial.printf("Connecting to WiFi %s\n", WIFI_SSID);
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(1000);
    // }
    // Serial.println("WiFi connected.");
    // WiFi.printDiag(Serial);
    // Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

    mqtt.begin();

    mqtt.subscribe("#", [](const char * topic, const char * payload) {
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });

    // mdns_responder = new MDNSResponder();
    // if (mdns_responder->begin("broker")) {
    //     log_i("MDNS responder started");
    // }
    // mdns_responder->addService("mqtt", "tcp", MQTT_TCP);
    // mdns_responder->addService("mqtt-ws", "tcp", MQTT_WS);
    // mdns_responder->addService("http", "tcp", HTTP_PORT);
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
        // log_d("Publishing message in topic '%s': %s", topic.c_str(), message.c_str());
        mqtt.publish(topic, message);
        mqtt.loop();
        
        topic = "esp32/free-heap";
        message = String(esp_get_free_heap_size());
        mqtt.publish(topic, message);

        last = millis();
    }
#if 0
    // Publish a greeting message every seconds
    if (millis() - last_publish_time >= 1000) {
        // We're publishing to a topic, which we're subscribed too.
        // The broker should deliver the messages back to us.

        // publish a literal flash string
        mqtt.publish("picomqtt/flash_string/literal", F("Literal PGM string"));

        // publish a string from a PGM global
        mqtt.publish_P("picomqtt/flash_string/global", flash_string);

        // The topic can be an F-string too:
        // mqtt.publish_P(F("picomqtt/flash_string/global"), flash_string);

        // publish binary data
        const char binary_payload[] = "This string could contain binary data including a zero byte";
        size_t binary_payload_size = strlen(binary_payload);
        mqtt.publish("picomqtt/binary_payload", (const void *) binary_payload, binary_payload_size);

        // Publish a big message in small chunks
        auto publish = mqtt.begin_publish("picomqtt/chunks", 1000);

        // Here we're writing 10 bytes 100 times
        for (int i = 0; i < 1000; i += 10) {
            publish.write((const uint8_t *) "1234567890", 10);
        }

        // In case of chunked published, an explicit call to send() is required.
        publish.send();

        last_publish_time = millis();
    }
#endif
    yield();
}
