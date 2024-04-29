#include <Arduino.h>
#include <ArduinoJson.h>

#include "stats.hpp"
#include "broker.hpp"

RunningStats gps_stats, alt_stats;

void stats_loop(void) {
    JsonDocument json;
    json["tick"] = micros();
    json["mean"] = alt_stats.Mean();
    json["stddev"] = alt_stats.StandardDeviation();
    json["ci95"] = alt_stats.ConfidenceInterval(CI95);

    json["gps-mean"] = gps_stats.Mean();
    json["gps-stddev"] = gps_stats.StandardDeviation();
    json["gps-ci95"] = gps_stats.ConfidenceInterval(CI95);


    auto publish = mqtt.begin_publish("altstats", measureJson(json));
    serializeJson(json, publish);
    publish.send();

}

void stats_setup(void) {
    
}