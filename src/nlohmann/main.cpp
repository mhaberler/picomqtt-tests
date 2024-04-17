#include <Arduino.h>
#include <nlohmann/json.hpp>
#include "hexdump.h"
using json = nlohmann::json;


void setup() {
    delay(3000);
    Serial.begin(115200);
    nlohmann::json doc;
    doc["hello"] = "world";
    json ex1 = json::parse(R"(
  {
    "pi": 3.141,
    "happy": true
  }
)");

    auto mp = json::to_msgpack(ex1);
    hexdump(Serial, mp.data(), mp.size());
    auto cb = json::to_cbor(ex1);
    hexdump(Serial, cb.data(), cb.size());

    json fcb = json::from_cbor(cb);
    auto str = fcb.dump(2, ' ', true);
    Serial.println(str.c_str());
    str = fcb.dump();
    Serial.println(str.c_str());
}

void loop() {
}



