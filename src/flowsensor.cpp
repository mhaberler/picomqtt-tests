

#include "params.hpp"
#include "prefs.hpp"
#include "pindefs.h"
#include "FunctionalInterrupt.h"
#include "fmicro.h"
#include "broker.hpp"
#include <ArduinoJson.h>

#include "FlowSensor.h"
#include "QuadratureDecoder.h"

#define T2OK(x) ((x) ? "OK" : "FAILED")

#ifdef FLOWSENSOR_AB
    #define PIN_FLOWSENSOR_A 13  // blue port
    #define PIN_FLOWSENSOR_B 14
#endif


static FlowSensor flow_sensor;
static QuadratureDecoder qdecoder;
static int32_t track_count;
static uint32_t track_now;
static float max_rate;

uint8_t flowsensor_A = PIN_FLOWSENSOR_A;
uint8_t flowsensor_B = PIN_FLOWSENSOR_B;

void flow_setup(void) {
#ifdef FLOWSENSOR
    flow_sensor.begin(flowsensor_A);
    log_i("flowsensor on %u enabled: %s", flowsensor_A, T2OK(flow_sensor.enable()));
#endif

#ifdef QUADRATURE_DECODER
    qdecoder.begin(flowsensor_A, flowsensor_B, 1, 0);
    log_i("A/B sensor on %u/%u enabled: %s", flowsensor_A, flowsensor_B, T2OK(qdecoder.enable()));
#endif
}

void flow_report(bool force) {

#ifdef QUADRATURE_DECODER
    qsensor_report_t report;
    qdecoder.getReport(report);

    if ((report.count != track_count) || force) {
        uint32_t now = micros();

        float delta = (float)(report.count - track_count);
        float rate = abs(delta * 1.e6f / (now - track_now));
        if (rate > max_rate) {
            max_rate = rate;
        }
        track_count = report.count;
        track_now = now;

        // log_i("count: %d delta: %.3f rate: %.3f force %d", report.count,
        //       delta, rate, force);

        JsonDocument json;
        json["time"] = fseconds();
        json["rate"] = rate * 10;
        json["count"] = report.count;
        auto publish = mqtt.begin_publish("flow", measureJson(json));
        serializeJson(json, publish);
        publish.send();
    }
#endif
#ifdef FLOWSENSOR
    flowsensor_report_t report;
    flow_sensor.getReport(report);

    if (flow_sensor.flowDetected() || force) {

        flow_sensor.getReport(report);
        // log_i("IRQs: %u  count: %u last_change: %u last_sample: %u force %d",
        //       report.count, report.last_change, report.last_sample, force);

        uint32_t now = micros();

        float delta = (float)(report.count - track_count);
        float rate = delta * 1.e6f / (now - track_now);
        if (rate > max_rate) {
            max_rate = rate;
        }
        track_count = report.count;
        track_now = now;

        JsonDocument json;
        json["time"] = fseconds();
        json["rate"] = rate * 10;
        json["count"] = report.count;
        auto publish = mqtt.begin_publish("flow", measureJson(json));
        serializeJson(json, publish);
        publish.send();
    }
#endif
}