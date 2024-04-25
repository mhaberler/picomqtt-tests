# lookup MQTT-over-websockets broker via MDNS
# connect & periodically publish a message
#
import socket
import time
from zeroconf import ServiceBrowser, Zeroconf
import paho.mqtt.client as mqtt
import random
import logging

logging.basicConfig(level=logging.DEBUG)

class MQTTBrokerListener:
    def __init__(self):
        self.services = {}
        self.zeroconf = Zeroconf()
        self.browser = ServiceBrowser(self.zeroconf, "_mqtt-ws._tcp.local.", self)

    def update_service(self, zc: "zeroconf.Zeroconf", type_: str, name: str) -> None:
        pass

    def add_service(self, zeroconf, type, name):
        info = zeroconf.get_service_info(type, name)
        if info:
            self.services[name] = info

    def remove_service(self, zeroconf, type, name):
        if name in self.services:
            del self.services[name]

    def get_brokers(self):
        brokers = []
        for name, info in self.services.items():
            port = info.port
            for adr in info.addresses:
                ip_addr = socket.inet_ntoa(adr)
                broker = {"ip": ip_addr, "port": port}
                brokers.append(broker)
        return brokers


def on_connect(mqttc, obj, flags, reason_code, properties):
    print("reason_code: " + str(reason_code))
    mqttc.subscribe("#", 0)


def on_publish(cclient, userdata, mid, reason_code, properties):
    print("Message published")


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


mqtt_listener = MQTTBrokerListener()

while True:
    brokers = mqtt_listener.get_brokers()
    if brokers:
        break
    time.sleep(1)

broker = brokers[0]  # Assuming there's only one broker
logging.info(f"MQTT broker at {broker['ip']}:{broker['port']}")

client_id = f"ps-{random.randint(0, 1000)}"

client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    transport="websockets",
    client_id=client_id,
    clean_session=True,
)
client.enable_logger()

client.on_connect = on_connect
client.on_message = on_message
client.connect(broker["ip"], broker["port"], 60)

topic = "blah"
n = 0
client.loop_start()

while True:
    client.publish(topic, f"Hello, MQTT! {n}")
    client.loop()
    time.sleep(1)
    n += 1
