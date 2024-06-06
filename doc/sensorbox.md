
## Using side: web client and/or custom devices
````mermaid

graph LR
mdns["Multicast\nDNS"] --server\nname\nannouncement--> clients
mdns  --server\nname\nannouncement--> device

webserver <--HTTP--> clients["Web \nclients "]
webserver["HTTP\nserver"] <--HTTP--> device
flash[("web app\nSD card")] <--> webserver

broker["MQTT\nbroker"] <--TCP--> device["custom\ndevice"]
broker["MQTT\nbroker"] <--websockets--> clients

````


## Sensorbox structure

````mermaid

graph LR
    scanner["BLE\n scanner"] -- adv --> decoder["BLE\n decoder"] --sensor\nvalues-->broker("MQTT\n broker")
    baro["pressure\nsensor 1..3"] --"pressure\n altitude\nvspeed"-->broker

gps["GPS"] --"loc\nGPS alt\nheading\nspeed"--> broker
gps--loc-->demlookup["DEM\nlookup"] --AGL-->broker
gps--alt\nloc-->airspacelookup["airspace\nlookup"]--current\nairspace\nstack-->broker
imu["IMU"] --> imudriver["IMU\n driver"]--"spatial\norientation &\ncompass"-->broker
nfc["NFC\nreader"] --> NFCdriver["NFC\n driver"]--tag info -->broker
nvs("Flash\nstorage")  <-->  persist["settings"] <--"operating parameters &\ncalibration"--> broker 

dem[("elevation\nmodel")] -->demlookup

airspace[("airspace\nshapes")]-->airspacelookup

mdns["Multicast\nDNS"] --server\nname\nannouncement-->clients
webserver["web\nserver"] <--HTTP--> clients["web clients\ndevices"]
broker--websockets or\nTCP-->clients
````

