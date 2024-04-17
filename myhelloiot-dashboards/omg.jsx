
<DashboardPage title="OpenMQTT Gateway example">
    <Card title="Ruuvi temperature">
        <ViewUnit
            topic="home/OMG_heltec_ble/BTtoMQTT/E691DF7BE54D"
            subconvert={JSONConvert(value => value["tempc"])}
        />
    </Card>


    <ViewCard
        title="OE-SOX Envelope"
        topic="home/OMG_heltec_ble/BTtoMQTT/E691DF7BE54D"
        subconvert={JSONConvert(value => value["tempc"])}
        format={LinearIconFormat({
            title: "temperature",
            ...Celsius(),
            step: 2
        })}
    />
    <ViewCard
        topic="home/OMG_heltec_ble/BTtoMQTT/E691DF7BE54D"
        subconvert={JSONConvert(value => value["hum"])}
        format={LinearIconFormat({
            title: "humidity",
            ...Percent(),
            step: 2
        })}
    />
</DashboardPage>