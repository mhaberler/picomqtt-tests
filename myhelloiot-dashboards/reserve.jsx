


<DashboardPage title="Balloon">
<Card title="Vertical speed">
    <ViewUnit
      topic="dps368-0"
      subconvert={JSONConvert(value => value["verticalSpeedKF"])}
    />
  </Card>
  
<Card title="Ruuvi temperature">
    <ViewUnit
      topic="dps368-0"
      subconvert={JSONConvert(value => value["verticalSpeedKF"])}
    />     
  </Card>

  <ViewCard
    title="Linear gauge card"
    topic="dps368-0"
    subconvert={JSONConvert(value => value["verticalSpeedKF"])}

    format={LinearIconFormat({
      title: "vertical speed",
      step: 1,
      min: -5.0,
      max: 5.0,
    })}
  />
  <ViewCard
    title="Dashboard gauge card"
    topic="dps368-0"
    subconvert={JSONConvert(value => value["verticalSpeedKF"])}
    format={DashboardIconFormat({
      title: "vertical speed",
      min: -5.0,
      max: 5.0
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
