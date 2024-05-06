{/* Gauges example. */}

<DashboardPage title="Gauges">
 <ViewCard 
title="Dial gauge card"
topic="dps-368-0"
subconvert={JSONConvert(value => value["v_baro"])}
format={DialIconFormat({
  title: "Dial gauge"
})}
/>
</DashboardPage>
