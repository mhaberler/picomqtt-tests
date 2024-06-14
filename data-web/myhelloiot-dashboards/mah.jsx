{/* Balloon  example. 
*/ }
<div>
    <DashboardPage title="Balloon status">

        <Card title="Baro">
            <ViewCard
                title="vertical speed"
                topic="baro/0"
                subconvert={JSONConvert(value => +value["verticalSpeedSmoothed"].toFixed(2))}
                format={FuelIconFormat({
                    title: "m/s",
                    min: -5.0,
                    max: 5.0,
                    startangle: 135,
                    endangle: 405,
                    labelstep: 0.5,
                    step: 1
                })}
            />
            <ViewCard
                title="Altitude"
                topic="baro/0"
                subconvert={JSONConvert(value => +value["altitudeSmoothed"].toFixed(1))}
                format={
                    NumberIconValueFormat({
                        style: "unit",
                        unit: "meter",
                        maximumFractionDigits: 1,
                        minimumFractionDigits: 0,
                    })
                }
            />

        </Card>

        <Card title="Envelope & OAT">
            <ViewUnit
                subtopic="ble/e691df7be54d"
                subconvert={JSONConvert(value => value["tempc"])}
                format={ProgressIconFormat({
                    title: "envelope temp",
                    ...Celsius(),
                    step: 2
                })}
            />
            <ViewUnit
                subtopic="ble/e691df7be54d"
                subconvert={JSONConvert(value => value["hum"])}
                format={ProgressIconFormat({
                    title: "envelope humidity",
                    ...Percent(),
                    step: 2
                })}
            />
            <ViewUnit
                subtopic="ble/dd79c68fbda2"
                subconvert={JSONConvert(value => value["tempc"])}
                format={ProgressIconFormat({
                    title: "OAT",
                    ...Celsius(),
                    step: 2
                })}
            />
            <ViewUnit
                subtopic="ble/dd79c68fbda2"
                subconvert={JSONConvert(value => value["hum"])}
                format={ProgressIconFormat({
                    title: "OA humidity",
                    ...Percent(),
                    step: 2
                })}
            />
        </Card>

        {/* <Card title="Tanks"> */}
        <Card title="Tanks">

            <ViewUnit
                title="Tank1"
                className="FOOO"
                topic="ble/d82cc3c65d32"
                subconvert={JSONConvert(value => value["level"])}
                format={ProgressIconFormat({
                    title: "level",
                    ...Percent(),
                    step: 2
                })}
            />
            <ViewUnit
                title="Tank2"
                topic="ble/f8eecc42af8d"
                subconvert={JSONConvert(value => value["level"])}
                format={ProgressIconFormat({
                    title: "level",
                    ...Percent(),
                })}
            />
            <ViewCard
                title="Battery"
                topic="system/battery"
                subconvert={JSONConvert(value => value["level"])}
                format={ProgressIconFormat({
                    title: "sensorbox",
                    ...Percent()
                })}
            />
            <ViewUnit
                subtopic="ble/e691df7be54d"
                subconvert={JSONConvert(value => value["batt"])}
                format={ProgressIconFormat({
                    title: "envelope sensor battery",
                    ...Percent(),
                })}
            />
            <ViewUnit
                subtopic="ble/dd79c68fbda2"
                subconvert={JSONConvert(value => value["batt"])}
                format={ProgressIconFormat({
                    title: "OAT sensor battery",
                    ...Percent(),
                })}
            />
        </Card>
        {/*  </Card> */}

    </DashboardPage >

    <DashboardPage title="Parameters">

        <Card title="smoothing alpha">
            <ViewUnit
                topic="preferences/baro/alpha"
                format={Percent()}
            />
            <SliderUnit
                topic="preferences/baro/alpha"
                puboptions={{ retain: true }}
                format={Percent()}

            />
        </Card>

    </DashboardPage>

</div>