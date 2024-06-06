{/* Balloon  example. */}

<DashboardPage title="Balloon status">

<ViewCard
    title="vertical speed 0"
    topic="baro/0"
    subconvert={JSONConvert(value =>  +value["verticalSpeedKF"].toFixed(2))}
    format={DashboardIconFormat({
        title: "vertical speed",
        min: -4.0,
        max: 4.0
    })}
/>
<ViewCard
    title="vertical speed 1"
    topic="baro/1"
    subconvert={JSONConvert(value =>  +value["verticalSpeedKF"].toFixed(2))}
    format={DashboardIconFormat({
        title: "vertical speed",
        min: -4.0,
        max: 4.0
    })}
/>

<Card title="Altitude">
    <ViewUnit
        topic="baro/0"
        subconvert={JSONConvert(value =>  +value["altitude"].toFixed(1))}
    />
</Card>

<Card title="OAT">
    <ViewUnit
        topic="ble/c7d888f2eb44"
        subconvert={JSONConvert(value => value["tempc"])}
    />
</Card>


<ViewCard
    title="Envelope temperature"
    topic="ble/e691df7be54d"
    subconvert={JSONConvert(value => value["tempc"])}
    format={LinearIconFormat({
        title: "temperature",
        ...Celsius(),
        step: 2
    })}
/>
<ViewCard
    title="Envelope humidity"
    topic="ble/e691df7be54d"
    subconvert={JSONConvert(value => value["hum"])}
    format={LinearIconFormat({
        title: "percent",
        ...Percent(),
        step: 2
    })}
/>
<ViewCard
    title="Tank1"
    topic="ble/d82cc3c65d32"
    subconvert={JSONConvert(value => value["level"])}
    format={LinearIconFormat({
        title: "level",
        ...Percent(),
        step: 2
    })}
/>
<ViewCard
    title="Tank2"
    topic="ble/f8eecc42af8d"
    subconvert={JSONConvert(value => value["level"])}
    format={LinearIconFormat({
        title: "level",
        ...Percent(),
        step: 2
    })}
/>
</DashboardPage>

