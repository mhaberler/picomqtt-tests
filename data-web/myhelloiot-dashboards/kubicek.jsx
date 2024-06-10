{/* Balloon  example. 
  */ }

<DashboardPage title="Balloon status">

    <Card title="Baro">
        {/* <ViewUnit
            title="vertical speed 0"
            topic="baro/0"
            subconvert={JSONConvert(value => +value["verticalSpeedKF"].toFixed(2))}
            format={FuelIconFormat({
                title: "vertical speed m/s",
                min: -4.0,
                max: 4.0,
                startangle: 135,
                endangle: 405,
                labelstep: 0.5,
            })}
        />
                <NotifyUnit title="notify"
            subtopic="notify"
        />
        <ViewUnit
            title="XXXX"
            topic="baro/0"
            subconvert={JSONConvert(value => +value["altitude"].toFixed(1))}
            format={
                NumberIconValueFormat({
                    style: "unit",
                    unit: "meter",
                    maximumFractionDigits: 1,
                    minimumFractionDigits: 0,
                })
            }
        /> */}
    </Card>

    <Card title="Envelope & OAT">
        <ViewUnit
            title="Envelope temperature"
            subtopic="ble/e691df7be54d"
            subconvert={JSONConvert(value => value["tempc"])}
            format={LinearIconFormat({
                title: "temperature",
                ...Celsius(),
                step: 2
            })}
        />
        <ViewUnit
            title="Envelope humidity"
            subtopic="ble/e691df7be54d"
            subconvert={JSONConvert(value => value["hum"])}
            format={LinearIconFormat({
                title: "rel hum",
                ...Percent(),
                step: 2
            })}
        />
        <ViewUnit
            subtopic="ble/c7d888f2eb44"
            subconvert={JSONConvert(value => value["hum"])}
            format={Percent()}
        />
        <ViewUnit
            subtopic="ble/c7d888f2eb44"
            subconvert={JSONConvert(value => value["tempc"])}
            format={Celsius()}
        />

        <NotifyUnit title="notify"
        /> 

    </Card>
    <Card title="Tanks">
        <ViewUnit
            title="Tank1"
            className="FOOO"
            topic="ble/d82cc3c65d32"
            subconvert={JSONConvert(value => value["level"])}
            format={LinearIconFormat({
                title: "level",
                ...Percent(),
                step: 2
            })}
        />
        <ViewUnit
            title="Tank2"
            topic="ble/f8eecc42af8d"
            subconvert={JSONConvert(value => value["level"])}
            format={LinearIconFormat({
                title: "level",
                ...Percent(),
                step: 2
            })}
        />

    </Card>
    {/*  </Card>

</DashboardPage >

