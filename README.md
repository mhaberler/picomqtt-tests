# MQTT Sensorbox
The purpose of this device is to:

- publish values of attached sensors via MQTT-over-Websockets and MQTT-over-TCP
- typical sensors would be barometric pressure, an IMU (accelerometer/gyro/magnetometer), and a GPS device
- provide optional postprocessing of sensor data 
- run an embedded MQTT broker with Websockets adapter
- publish advertisement-based BLE sensor reports as supported by the [Theengs library](https://github.com/theengs/decoder)
- run an embedded webserver to host and serve web applications
- read NFC tags, and publish their contents via MQTT for application use
- provide surface elevation given a geographic coordinate, via accessing digital elevation models stored on an SD card
- offer a way to manage and edit files stored in an embedded filesystem via a web browser
- export configuration values as MQTT topics backed by non-volatile storage for persistence
- provide a web configuration portal to set WiFi credentials, upload files to the  embedded filesystem and update the firmware
- run completely isolated without any Internet connectivcity

A typical use case would be Sensorbox connecting to a mobile's WiFi hotspot and offer its services to the mobile and other Wifi clients of the mobile's WiFi hotspot.

## Usage Overview
````mermaid

graph LR

sensors["sensors"] --publish--> broker
broker["MQTT\nbroker"] <--publish\nsubscribe\nMQTT-websockets--> clients
webserver <--HTTP--> clients["Web \nclients "]

littlefs[("web apps+\nassets\nLittleFS")] <--> webserver
flash[("web apps+\nassets\nSD card")] <--> webserver


mdns["Multicast\nDNS"] --server name\nannouncement--> clients

````

## Supported protocols and features

- HTTP service on port 80
- MQTT-over-Websockets 
- MQTT-over-TCP
- Host(.local) and service announcements via mDNS and [SSDP](https://github.com/luc-github/ESP32SSDP#2.0.0) (aka "Universal Plug and Play")
- configuration via NVS-persistent topics - see [PicoSettings](https://github.com/mhaberler/PicoSettings)
- NFC reader support including Type 4 tags, like the Ruuvi Sensor - see repos https://github.com/mhaberler/NDEF https://github.com/mhaberler/Arduino_MFRC522v2 
- Digital Elevation Models in pmtiles format, see [this project](https://github.com/mhaberler/embedded-protomaps) for hints to generate your own


## Component Overview

````mermaid
graph LR
    scanner["BLE\n scanner"] -- adv --> decoder["Theengs\n decoder"] --sensor\nvalues-->broker("MQTT\n broker")
    baro["pressure\nsensors"] -- "pressure\nbarometric altitude" -->broker
    baro --> Kalman\nestimate -- "vspeed\nvaccel" --> broker

gps["GPS"] --"location\nGPS altitude\nheading\nspeed"--> broker
gps--loc-->demlookup["elevation\nlookup"] --surface\nelevation-->broker
gps--alt\nloc-->airspacelookup["airspace\nlookup"]--current\nairspace\nstack-->broker
imu["inertial\nmeasurement\nunit"] --"spatial\norientation &\ncompass"-->broker
nfc["NFC\nreader"] -- tag\ninfo -->broker
nvs("Flash\nstorage")  <-->  persist["settings"] <--"operating parameters &\ncalibration"--> broker 

dem[("elevation\nmodels")] -->demlookup

airspace[("airspace\nshapes")]-->airspacelookup
broker--websockets or\nTCP-->clients
webserver["web\nserver"] <--HTTP--> clients["web clients\ndevices"]
mdns["Multicast\nDNS"] --server\nname\nannouncement-->clients
````

## Platform and Supported Hardware

- ESP32S3 (preferrably with PSRAM) and sufficient flash memory for the embedded filesystem (LittleFS)
- ESP32C3
- DPS368 barometric pressure sensors, via I2C, interrupt-driven
- ICM20948 IMU, via I2C, interrupt-driven
- GPS: Ublox models M9N M8P M8Q F9P, via I2C
- any BLE sensor supported by [Theengs](https://decoder.theengs.io/devices/devices.html)

## Embedded applications (part of the repo)

- a localized version of [MyhelloIOT](https://github.com/adrianromero/myhelloiot)
- the web version of the [MQTTX MQTT browser](https://github.com/emqx/MQTTX)
- [device-orientation](https://github.com/mhaberler/device-orientation): a simple threejs 3D application to visualize the IMU orientation

## Implementation status

all features except airspace lookup

## Limitations

- no SSL support 
- assumed to run in a trusted environment

## Parts used 
- [PicoMQTT](https://github.com/mlesniew/PicoMQTT)  and [PicoWebsockets](https://github.com/mlesniew/PicoWebsocket) by Michał Leśniewski
- [PicoSettings](https://github.com/mhaberler/PicoSettings) by myself
- [esp-fs-webserver](https://github.com/cotestatnt/esp-fs-webserver) by Tolentino Cotesta
- [Theengs decopder](https://github.com/theengs/decoder) from the [OpenMQTT Gateway](https://docs.openmqttgateway.com/) project
- [ArduinoJson](https://arduinojson.org/) by  Benoît Blanchon
- an embedded version of [PMTiles](https://github.com/protomaps/PMTiles) by, and with the help of Brandon Liu

## Hardware setup

todo

## Client Test Results
the following clients were successfully tested against PicoMQTT/PicoWebsockets and should work with SenorBox:

### MQTT-Websockets
- Sensor-Logger Android 1.31.4 build 3145890 
- Sensor-Logger iOS 1.32 build 4
- EasyMQTT iOS 1.16.1b503 
- myhelloiot 1.1.5
- IoT MQTT Panel 2.14.58
- browser: MQTT.js 5.5.4 - see data/mqtt.html
- browser: paho-mqtt 1.1.0 - see data/paho.html
- nodejs: MQTT.js 5.5.4 - see nodejs/mqtt_ws.js
- Python https://pypi.org/project/paho-mqtt/ - see python/*.py


### MQTT-over-TCP
- OpenMQTT Gateway - development, commit f278555
- node-red Node-RED version: v3.1.9
- mymqtt.app Android 2.3.3 (7507)
- mymqtt.app iOS 1.1.1 (3)
- EasyMQTT iOS 1.16.1b503 
- IoT MQTT Panel 2.14.58
- Python https://pypi.org/project/paho-mqtt/ - see python/*.py
- nodejs: MQTT.js 5.5.4 - see nodejs/mqtt_tcp.js

### failed 
- MQTT board - requires TLS  https://mqttboard.flespi.io/#/
- MQTT Tiles - requires TLS  https://mqtttiles.flespi.io/#/
- MQTT.cool - requires TLS   https://testclient-cloud.mqtt.cool/
