
<DashboardPage title="ESP32 Embedded MQTT Broker example">
    <Card title="CPU temperature">
        <ViewUnit
            topic="picomqtt/esp-cpu-temperature"
        />
    </Card>

    <Card title="Free heap">
        <ViewUnit
            topic="esp32/free-heap"
        />
    </Card>

</DashboardPage>