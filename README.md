# picomqtt-websockets example

this repo is for exploring and testing the PicoMQTT/PicoWebsockets.


## test results
the following clients were successfully tested against PicoMQTT/PicoWebsockets:

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
