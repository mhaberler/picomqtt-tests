const mqtt = require("mqtt");
const mdns = require("mdns");

const browser = mdns.createBrowser(mdns.tcp('mqtt-ws'));

const subtopic = "#";
const pubtopic = "foobar";
var found = false;

browser.on('serviceUp', service => {
    console.log("Found MQTT service:", service.host, service.port);
    if (found)  // once only
        return;
    found = true;

    const client = mqtt.connect(`ws://${service.host}:${service.port}`);

    client.on('connect', () => {
        console.log('Connected to MQTT broker');

        client.subscribe(subtopic);

        setInterval(() => {
            console.log("publish");
            client.publish(pubtopic, 'Hello from MQTT client');
        }, 2000);
    });

    client.on("message", function (topic, message) {
        console.log('got: topic ' + topic + " " + message);
    });

    client.on('error', err => {
        console.error('Error connecting to MQTT broker:', err);
    });
});

browser.start();


