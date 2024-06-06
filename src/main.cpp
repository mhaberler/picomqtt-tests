
#ifdef M5UNIFIED
    #include <M5Unified.h>
#else
    #include <Arduino.h>
#endif

#include <Wire.h>
#include <PicoMQTT.h>
#include <PicoWebsocket.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "tickers.hpp"
#include "i2cio.hpp"
#ifdef DEM_SUPPORT
    #include "demlookup.hpp"
#endif
#include "sensor.hpp"
#include "params.hpp"
#include "prefs.hpp"
#include "fmicro.h"

#include "pindefs.h"

#if __has_include("myconfig.h")
    #include "myconfig.h"
#endif


void webserver_setup(void);
void webserver_loop(void);
void sensor_setup(void);
void sensor_loop(void);
void irq_setup(void);

::WiFiServer mqtt_tcp_server(MQTT_TCP);
::WiFiServer mqtt_ws_server(MQTT_WS);
PicoWebsocket::Server<::WiFiServer> websocket_server(mqtt_ws_server);
PicoMQTT::Server mqtt(mqtt_tcp_server, websocket_server);

TICKER(internal, INTERVAL);
TICKER(deadman, DEADMAN_INTERVAL);


void setup() {

    delay(3000);
#ifdef M5UNIFIED
    auto cfg = M5.config();
    // cfg.output_power = true;
    M5.begin(cfg);
#endif
    Serial.begin(115200);
    // Serial.setDebugOutput(true);

    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    RGBLED(64,0,0);


#if defined(ARDUINO_M5STACK_CORES3) ||  defined(ARDUINO_M5STACK_Core2)
    Wire.begin();
    Wire.setClock(I2C_400K);
    Wire1.begin();
    Wire1.setClock(I2C_400K);
#if defined(DPS0_IRQ_PIN)
    pinMode(DPS0_IRQ_PIN, INPUT_PULLUP);
#endif
#if defined(DPS1_IRQ_PIN)
    pinMode(DPS1_IRQ_PIN, INPUT_PULLUP);
#endif
#if defined(DPS2_IRQ_PIN)
    pinMode(DPS2_IRQ_PIN, INPUT); // has external pulldown 33k
#endif
    pinMode(17, INPUT_PULLDOWN);
    pinMode(18, INPUT_PULLDOWN);
    delay(10);
    i2c_scan(Wire);
    i2c_scan(Wire1);

#endif
#if defined(DEVKITC)

    Wire.begin(I2C0_SDA, I2C0_SCL, I2C0_SPEED);
    Wire1.begin(I2C1_SDA, I2C1_SCL, I2C1_SPEED);

    pinMode(DPS0_IRQ_PIN, INPUT_PULLUP);
    pinMode(DPS1_IRQ_PIN, INPUT);
    pinMode(DPS2_IRQ_PIN, INPUT_PULLUP);
    pinMode(IMU_IRQ_PIN, INPUT_PULLUP);

    pinMode(TRIGGER1, OUTPUT);
    pinMode(TRIGGER2, OUTPUT);

    // digitalWrite(TRIGGER1, 1);
    // digitalWrite(TRIGGER1, 0);
    // digitalWrite(TRIGGER2, 1);
    // digitalWrite(TRIGGER2, 0);

    i2c_scan(Wire);
    i2c_scan(Wire1);
#endif
    settings_setup();
    webserver_setup();

#ifdef DEM_SUPPORT
    dem_setup();
#endif
    irq_setup();
    sensor_setup();

    mqtt.begin();

    mqtt.subscribe("system/interval", [](const char * topic, const char * payload) {
        uint32_t new_interval = strtoul (payload, NULL, 0);
        if (new_interval  > MIN_INTERVAL) {
            CHANGE_TICKER(internal, new_interval);
            Serial.printf("changed ticker to %u ms\n", new_interval);
        }
    });
    mqtt.subscribe("system/reboot",  [](const char * topic, const char * payload) {
        ESP.restart();
    });

    mqtt.subscribe("baro/reinit",  [](const char * topic, const char * payload) {
        int n = atoi(payload);
        if (n < 0)
            return;
        if (n > 2)
            return;
        int16_t ret = dps368_setup(n);
        log_i("reinit %d: %d", n, ret);
    });
    RUNTICKER(internal);
    RUNTICKER(deadman);
}


void loop() {
    mqtt.loop();
    webserver_loop();
    sensor_loop();

    if (TIME_FOR(internal)) {
        mqtt.publish("system/interval", String(internal_update_ms));
        mqtt.publish("system/free-heap", String(ESP.getFreeHeap()));
        mqtt.publish("system/reboot", "0");
        mqtt.publish("baro/reinit", "-1");
#ifdef DEM_SUPPORT
        publishDems();
#endif
#ifdef M5UNIFIED
        JsonDocument json;
        json["time"] = fseconds();
        json["level"] = M5.Power.getBatteryLevel();
        json["status"] = (int) M5.Power.isCharging();

        switch (M5.Power.isCharging()) {
            case m5::Power_Class::is_discharging:
                json["text"]  = "discharging";
                break;
            case m5::Power_Class::is_charging:
                json["text"]  = "charging";
                break;
            case m5::Power_Class::charge_unknown:
                json["text"]  = "unknown";
                break;
        }

        auto publish = mqtt.begin_publish("system/battery", measureJson(json));
        serializeJson(json, publish);
        publish.send();
#endif
        settings_tick();

        DONE_WITH(internal);
    }
    yield();
}
