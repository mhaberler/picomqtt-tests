
## Using side: web client and/or custom devices
````mermaid

graph LR


webserver <--HTTP--> clients["Web \nclients "]

littlefs("web apps+\nassets\nLittleFS in flash") <--> webserver
flash[("web apps+\nassets\nSD card")] <--> webserver
sensors["sensors"] --publish--> broker
broker["MQTT\nbroker"] <--publish\nsubscribe\nMQTT-websockets--> clients

mdns["Multicast\nDNS"] --server name\nannouncement--> clients


````
webserver["HTTP\nserver"] <--HTTP--> device
mdns  --server\nname\nannouncement--> device
broker["MQTT\nbroker"] <--TCP--> device["custom\ndevice"]


## Sensorbox structure

````mermaid
graph LR
    scanner["BLE\n scanner"] -- adv --> decoder["Theengs\n decoder"] --sensor\nvalues-->broker("MQTT\n broker")
    baro["pressure\nsensor 1..3"] --"pressure\n altitude\nvspeed\nvaccel"-->broker

gps["GPS"] --"loc\nGPS alt\nheading\nspeed"--> broker
gps--loc-->demlookup["DEM\nlookup"] --AGL-->broker
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

