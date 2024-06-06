


<DashboardPage title="Balloon">

  <ViewCard
    title="Linear gauge card"
    topic="dps368-0"
    subconvert={JSONConvert(value => value["verticalSpeedKF"])}

    format={LinearIconFormat({
      title: "vertical speed",
      unit: "meter",
      maximumSignificantDigits: 1,
      min: -1.0,
      max: 1.0,
    })}
  />
  <ViewCard
    title="Dashboard gauge card"
    topic="dps368-0"
    subconvert={JSONConvert(value => value["verticalSpeedKF"])}
    format={DashboardIconFormat({
      title: "vertical speed",
      min: -1.0,
      max: 1.0
    })}
  />
  <Card title="CPU temperature">
    <ViewUnit
      topic="esp32/esp-cpu-temperature"
    />
  </Card>

  <Card title="Free heap">
    <ViewUnit
      topic="esp32/free-heap"
    />
  </Card>

  <Card title="Lights">
    <ButtonUnit
      pubtopic="shellies/shellyswitch01/relay/1/command"
      subtopic="shellies/shellyswitch01/relay/1"
      format={SwitchIconValueFormat({ onoff: ONOFFStr })}
    />
  </Card>


</DashboardPage>
