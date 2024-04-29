#include <Arduino.h>
#include "freertos/ringbuf.h"

#include <NimBLEDevice.h>
#include "ruuvi.h"
#include "mopeka.h"
#include "tpms.h"
#include "otodata.h"
#include "hexdump.h"
#include "ringbuffer.hpp"
#include "broker.hpp"
#include "util.hpp"

#ifndef BLE_ADV_QUEUELEN
    #define BLE_ADV_QUEUELEN 2048
#endif
// move queue to PSRAM if possible
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 1) && defined(BOARD_HAS_PSRAM)
    #define BLE_ADV_QUEUE_MEMTYPE MALLOC_CAP_SPIRAM
#else
    #define BLE_ADV_QUEUE_MEMTYPE MALLOC_CAP_INTERNAL
#endif
#define BLE_NAME_SIZE 32

typedef struct  {
    uint64_t mac64;
    uint16_t mfid;
    int8_t rssi;
    char name[BLE_NAME_SIZE];
    uint8_t msg_size;
    uint8_t message[0];
} bleAdvMsgHdr_t;

static espidf::RingBuffer *bleadv_queue;
static uint32_t queue_full;
static NimBLEAdvertisedDevice *advDevice;
static  NimBLEScan *pScan;
static uint32_t scanTime = 3600 * 1000; // In seconds, 0 = scan forever
static long unsigned int last_millis;
static unsigned acquire_fail;

void report_ruuvi(const NimBLEAddress &mac, const ruuviAd_t &r) {
    // Serial.printf("ruuvi:  %s: temp=%f hum=%f press=%f rssi=%d\n", mac.toString().c_str(),
    //               r.temperature, r.humidity,r.pressure, r.rssi);
    JsonDocument json;
    json = r;
    json["type"] = "ruuvi";
    auto publish = mqtt.begin_publish(bleTopic(mac), measureJson(json));
    serializeJson(json, publish);
    publish.send();
}

void report_mopeka(const NimBLEAddress &mac,const mopekaAd_t &m) {
    // Serial.printf("mopeka:  %s: level=%d temp=%d quality=%u rssi=%d\n", mac.toString().c_str(),
    //               m.level, m.temperature, m.qualityStars, m.rssi);

    JsonDocument json;
    json = m;
    json["type"] = "mopeka";
    auto publish = mqtt.begin_publish(bleTopic(mac), measureJson(json));
    serializeJson(json, publish);
    publish.send();
}

void report_tpms(const NimBLEAddress &mac,const tpmsAd_t &t) {
    // Serial.printf("tpms: %s:  press=%f temp=%f bat=%u%% loc=%u rssi=%d\n", mac.toString().c_str(),
    //               t.pressure, t.temperature, t.batpct, t.location, t.rssi);
    JsonDocument json;
    json = t;
    json["type"] = "tpms";
    auto publish = mqtt.begin_publish(bleTopic(mac), measureJson(json));
    serializeJson(json, publish);
    publish.send();
}

void report_otodata(const NimBLEAddress &mac,const otodataAd_t &t) {
    // Serial.printf("otodata: %s: ", mac.toString().c_str());
    // if (t.has_level) {
    //     Serial.printf("level=%.1f%% status=%u ", t.level / 100.0, t.status);
    // }
    // if (t.has_serial) {
    //     Serial.printf("serial=%u model=%u ", t.serial, t.model);
    // }
    // Serial.printf("rssi=%d\n", t.rssi);
    JsonDocument json;
    json = t;
    json["type"] = "otodata";
    auto publish = mqtt.begin_publish(bleTopic(mac), measureJson(json));
    serializeJson(json, publish);
    publish.send();
}

int16_t getInt16(const uint8_t *data, int index) {
    return (int16_t)((data[index] << 8) + (data[index + 1]));
}

uint16_t getUint16(const uint8_t *data, int index) {
    return (uint16_t)((data[index] << 8) + (data[index + 1]));
}

int32_t getInt32(const uint8_t *data, int index) {
    return (int32_t)(
               (data[index] << 24) |
               (data[index+1] << 16) |
               (data[index+2] << 8) |
               (data[index+3]));
}

uint32_t getUint32(const uint8_t *data, int index) {
    return (uint32_t)(
               (data[index] << 24) |
               (data[index+1] << 16) |
               (data[index+2] << 8) |
               (data[index+3]));
}

int32_t getInt32LE(const uint8_t *data, int index) {
    return (int32_t)(
               (data[index]) |
               (data[index+1] << 8) |
               (data[index+2] << 16) |
               (data[index+3] << 24));
}

uint32_t getUint32LE(const uint8_t *data, int index) {
    return (uint32_t)(
               (data[index]) |
               (data[index+1] << 8) |
               (data[index+2] << 16) |
               (data[index+3] << 24));
}


uint16_t getUint16LE(const uint8_t *data, int index) {
    return (uint32_t)(
               (data[index]) |
               (data[index+1] << 8));
}

uint8_t getUint8(const uint8_t *data, int index) {
    return (uint8_t)((data[index]));
}

int8_t getInt8(const uint8_t *data, int index) {
    return (int8_t)((data[index]));
}

static void DecodeV5(const uint8_t *data, ruuviAd_t &ra) {
    int16_t i16;
    uint16_t u16;
    uint8_t u8;

    ra.availability = 0;
    ra.ruuvi_format = 5;

    i16 = getInt16(data, 3);
    if (i16 !=0x8000) ra.availability |= RUUVI_TEMPERATURE_AVAILABLE;
    ra.temperature = (float) i16 * 0.005;

    u16 = getUint16(data, 5);
    if (u16 != 65535) ra.availability |= RUUVI_HUMIIDTY_AVAILABLE;
    ra.humidity = (float)u16 * 0.0025;

    u16 = getUint16(data, 7);
    if (u16 != 65535) ra.availability |= RUUVI_PRESSURE_AVAILABLE;
    ra.pressure = ((float)u16 + 50000.0) / 100.0;

    ra.accelX = getInt16(data, 9);
    if (ra.accelX != 0x8000) ra.availability |= RUUVI_ACCELX_AVAILABLE;
    ra.accelY = getInt16(data, 11);
    if (ra.accelY != 0x8000) ra.availability |= RUUVI_ACCELY_AVAILABLE;
    ra.accelZ = getInt16(data, 13);
    if (ra.accelZ != 0x8000) ra.availability |= RUUVI_ACCELZ_AVAILABLE;

    u16 = data[15] << 3 | data[16];
    if (u16 != 2047) ra.availability |= RUUVI_BATTERY_AVAILABLE;
    ra.voltage = u16 + 1600;

    u8 = data[16] & 0x1F;
    if (u8 != 31) ra.availability |= RUUVI_TXPOWER_AVAILABLE;
    ra.power = (u8) * 2 - 40;

    u8 = getUint8(data, 17);
    if (u8 != 255) ra.availability |= RUUVI_MOVEMENT_AVAILABLE;
    ra.moveCount = u8;

    u16 = getUint16(data, 18);
    if (u16 != 65535) ra.availability |= RUUVI_SEQUENCE_AVAILABLE;
    ra.sequence = u16;
}

bool bleDeliver(const bleAdvMsgHdr_t *msg, size_t size) {

    NimBLEAddress mac = NimBLEAddress(msg->mac64);
    const uint8_t *data = msg->message;
    const uint8_t len = msg->msg_size;
    if (strlen((char *)msg->name)) {
        log_i("name: '%s'", msg->name);
    }
    // hexdump(Serial, data, len);
    switch (msg->mfid) {
        case 0x0499: { // Ruuvi manufacturer ID
                ruuviAd_t ruuvi_report = {};

                ruuvi_report.rssi = msg->rssi;
                if (data[2] == 0x5 && len > 19) {
                    DecodeV5(data, ruuvi_report);
                    report_ruuvi(mac, ruuvi_report);
                    return true;
                }
                if (data[2] == 0x3 && len > 15) {
                    log_e("upgrade your RuuviTag, this is still Version3 format");
                    return false;
                }
                log_e("failed to decode ruuvi msg");
                return false;
            }
        case 0x0059: { // Mopeka manufacturer ID
                mopekaAd_t mopeka_report = {};

                if (len != 12) {
                    log_e("Mopeka PRO: manufacturer data len (%u - expect 12)",
                          len);
                    return false;
                }

                mopeka_report.battery = (data[3] & 0x7f) / 32.0;
                mopeka_report.syncPressed = (data[4] & 0x80) > 0;
                mopeka_report.raw_temp = (data[4] & 0x7f);
                mopeka_report.temperature = mopeka_report.raw_temp - 40; // °C
                mopeka_report.qualityStars = (data[6] >> 6);

                mopeka_report.acceloX = data[10];
                mopeka_report.acceloY = data[11];

                mopeka_report.raw_level = ((int(data[6]) << 8) + data[5]) & 0x3fff;
                mopeka_report.level = mopeka_report.raw_level *
                                      (MOPEKA_TANK_LEVEL_COEFFICIENTS_PROPANE_0 +
                                       (MOPEKA_TANK_LEVEL_COEFFICIENTS_PROPANE_1 * mopeka_report.raw_temp) +
                                       (MOPEKA_TANK_LEVEL_COEFFICIENTS_PROPANE_2 * mopeka_report.raw_temp *
                                        mopeka_report.raw_temp));
                mopeka_report.rssi = msg->rssi;
                report_mopeka(mac, mopeka_report);
                return true;

            }

        case 0x0100: { // TPMS manufacturer ID variant 1
                tpmsAd_t tpms_report = {};
                if (len == 18) {
                    tpms_report.location = getUint8(data, 2) & 0x7f;
                    tpms_report.pressure = (float)getInt32LE(data, 8) / 100000.0;
                    tpms_report.temperature = (float)getInt32LE(data, 12) / 100.0;
                    tpms_report.batpct = getUint8(data, 16);
                    tpms_report.status = getUint8(data, 17);
                    tpms_report.rssi = msg->rssi;
                    report_tpms(mac, tpms_report);
                    return true;
                }
                return false;
            }
        case 0x00AC: { // TPMS manufacturer ID variant 2
                tpmsAd_t tpms_report = {};
                if (len == 15) {
                    tpms_report.pressure = getInt32LE(data, 0);
                    tpms_report.temperature = k0 + getInt32LE(data, 4) / 100.0;
                    tpms_report.batpct = getUint8(data, 5);
                    tpms_report.location = getUint8(data, 6) & 0x7f;
                    tpms_report.status = 0;
                    tpms_report.rssi = msg->rssi;
                    report_tpms(mac, tpms_report);
                    return true;
                }
                return false;
            }
        case 0x03B1: {
                // log_i("tank sensor %s", mac.toString().c_str());
                // hexdump(Serial, data, len);
                otodataAd_t oto_report = {};
                oto_report.rssi = msg->rssi;
                if (msg->msg_size == 21) {
                    oto_report.level = getUint16LE(data, 11);
                    oto_report.status = getUint16LE(data, 13);
                    oto_report.has_level = true;
                }
                if (msg->msg_size == 24) {
                    oto_report.serial = getUint32LE(data, 9);
                    oto_report.model = getUint32LE(data, 21);
                    oto_report.has_serial = true;
                }
                report_otodata(mac, oto_report);
                return false;
            }
        default:
            break;
    }
    return false;
}

class scanCallbacks : public NimBLEScanCallbacks {

    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->haveManufacturerData()) {

            const uint8_t *data =
                (const uint8_t *)advertisedDevice->getManufacturerData().data();
            size_t len = advertisedDevice->getManufacturerData().length();

            size_t total = sizeof(bleAdvMsgHdr_t) + len;
            bleAdvMsgHdr_t *ble_adv = nullptr;

            uint16_t mfid = data[1] << 8 | data[0];
            switch (mfid) {
                // filter ads for interesing sensors
                case 0x0499:  // Ruuvi manufacturer ID
                case 0x0059:  // Mopeka manufacturer ID
                case 0x0100:  // TPMS manufacturer ID variant 1
                case 0x00AC:  // TPMS manufacturer ID variant 2
                case 0x03B1:  // Otodata
                    if (bleadv_queue->send_acquire((void **)&ble_adv, total, 0) != pdTRUE) {
                        acquire_fail++;
                        return;
                    }
                    memset(ble_adv, 0, total);
                    ble_adv->mfid = mfid;
                    ble_adv->msg_size = len;
                    ble_adv->mac64 = (uint64_t) advertisedDevice->getAddress();
                    ble_adv->rssi = advertisedDevice->getRSSI();
                    memcpy(ble_adv->message, data, len);
                    if (advertisedDevice->haveName()) {
                        std::string name = advertisedDevice->getName();
                        strcpy(static_cast<char *>(ble_adv->name), name.c_str());
                    }
                    if ( bleadv_queue->send_complete(ble_adv) != pdTRUE) {
                        queue_full++;
                    }
                    break;
                default:
                    ;
            }
        }
    }
    void
    onScanEnd(NimBLEScanResults results) {
        log_i("\nonScanEnd: restart\n");
        NimBLEDevice::getScan()->start(scanTime, false);
    }
};

void process_ble(void) {
    if (!pScan->isScanning()) {
        pScan->start(scanTime, false);
    }
    size_t sz = 0;
    void *p = bleadv_queue->receive(&sz, 0);
    if (p == nullptr)
        return;
    bleDeliver((const bleAdvMsgHdr_t *)p, sz);
    bleadv_queue->return_item(p);
}

void setup_ble(void) {
    bleadv_queue = new espidf::RingBuffer();
    bleadv_queue->create(BLE_ADV_QUEUELEN, RINGBUF_TYPE_NOSPLIT);
    NimBLEDevice::init("");
    pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new scanCallbacks());
    pScan->setInterval(97);
    pScan->setWindow(67);
    pScan->setMaxResults(0);
    pScan->setActiveScan(false);
    pScan->setDuplicateFilter(false);
    pScan->start(scanTime, false);
}

#if defined(BLE_ARDUINO_MAIN)
void loop() {
    process_ble();
    yield();
}

void setup() {
    delay(3000);
    Serial.begin(115200);
    log_i("Starting NimBLE Scanner");
    setup_ble();
}
#endif