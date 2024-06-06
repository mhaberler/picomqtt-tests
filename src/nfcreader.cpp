
#ifdef NFC_SUPPORT
#ifdef M5UNIFIED
    #include <M5Unified.h>
#else
    #include <Arduino.h>
#endif
#include "params.hpp"
#include "sensor.hpp"
#include <MFRC522DriverI2C.h>
#include <Wire.h>

#include "NdefMessage.h"
#include "NfcAdapter.h"
#include <MFRC522Debug.h>
#include <MFRC522Extended.h>
#include <MFRC522v2.h>

#include <SPI.h>
#include <stdlib.h>

#include "nfc_input.h"
#include "i2cio.hpp"
#include "broker.hpp"
#include "fmicro.h"

using StatusCode = MFRC522Constants::StatusCode;

extern nfc_reader_t nfcconf;
static bwTagType_t analyseTag(NfcTag &tag, JsonDocument &doc);

static const char *ruuvi_ids[] = {
    "\002idID: ",
    "\002adMAC: ",
    "\002swSW: ",
    "\002dt",
};

void nfc_setup(void) {
    if (!nfcconf.driver_instantiated) {
        nfcconf.driver = new MFRC522DriverI2C{nfcconf.dev.i2caddr, *nfcconf.wire};
        nfcconf.mfrc522 = new MFRC522Extended{*nfcconf.driver};
        nfcconf.nfc = new NfcAdapter(nfcconf.mfrc522);
        nfcconf.key = new MFRC522::MIFARE_Key{{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
        log_i("NFC driver instantiated");
        // nfcconf.dev.device_present = detect(*nfcconf.wire, nfcconf.dev.i2caddr);
        nfcconf.driver_instantiated = true;
    }
}

bool nfc_reader_present(void) {
    return  (nfcconf.dev.device_initialized && nfcconf.dev.device_present);
}

void nfc_loop(void) {
    post_softirq(&nfcconf);
}

void nfc_poll(void) {
    nfcconf.dev.device_present = detect(*nfcconf.wire, nfcconf.dev.i2caddr);
    if (nfcconf.dev.device_present ^ nfcconf.dev.device_initialized) {
        if (nfcconf.dev.device_present) {
            // just plugged in
            nfcconf.mfrc522->PCD_Init();  // causes useless Wire.begin()
            nfcconf.nfc->begin();
            nfcconf.nfc->setMifareKey(nfcconf.key);

            MFRC522::PCD_Version version = nfcconf.mfrc522->PCD_GetVersion();
            log_i("RFID reader detected, firmware version: 0x%x", version);
            mqtt.publish("nfc/reader", "1");
            nfcconf.dev.device_initialized = true;
        } else {
            mqtt.publish("nfc/reader", "0");
            if (nfcconf.dev.device_initialized)
                log_e("RFID reader was unplugged");
            nfcconf.dev.device_initialized = false;
            return;
        }
    }
    if (nfcconf.dev.device_initialized && nfcconf.dev.device_present) {
        if (nfcconf.nfc->tagPresent()) {
            log_i("Reading NFC tag");
            NfcTag tag = nfcconf.nfc->read();

            JsonDocument jsondoc;
            bwTagType_t type = analyseTag(tag, jsondoc);
            jsondoc["um"] = type;
            jsondoc["time"] = fseconds();
            log_i("analyseTag=%d", type);

            tag.tagToJson(jsondoc);
            auto publish = mqtt.begin_publish("nfc/tag", measureJson(jsondoc));
            serializeJson(jsondoc, publish);
            publish.send();

            nfcconf.nfc->haltTag();
            nfcconf.nfc->setMifareKey(nfcconf.key);
        }
    }
}

static bwTagType_t
analyseTag(NfcTag &tag, JsonDocument &doc) {
    if (!tag.hasNdefMessage()) {
        return BWTAG_NO_MATCH;
    }
    uint8_t nrec = tag.getNdefMessage().getRecordCount();
    NfcTag::PICC_Type tagType = tag.getTagType();

    switch (tagType) {

        case  MFRC522Constants::PICC_TYPE_ISO_14443_4: {
                if (nrec == 4) {
                    String content[4];
                    for (auto i = 0; i < nrec; i++) {
                        NdefRecord record = tag.getNdefMessage()[i];
                        const byte *payload = record.getPayload();
                        size_t prefix_len = strlen((const char *)ruuvi_ids[i]);
                        if (payload == NULL) {
                            return BWTAG_NO_MATCH;
                        }
                        if (memcmp(payload, ruuvi_ids[i], prefix_len) != 0) {
                            return BWTAG_NO_MATCH;
                        }
                        content[i] = String(record.getPayload() + prefix_len,
                                            record.getPayloadLength() - prefix_len);
                    }
                    // if we made it here, it's a Ruuvi tag
                    auto ruuvi = doc["payload"].to<JsonObject>();
                    ruuvi["ID"] = content[0];
                    ruuvi["MAC"] = content[1];
                    ruuvi["SW"] = content[2];
                    // skip the mystery 'dt' record
                    return BWTAG_RUUVI;
                }
            }
            break;

        case MFRC522Constants::PICC_TYPE_MIFARE_4K:
        case MFRC522Constants::PICC_TYPE_MIFARE_1K:
        case MFRC522Constants::PICC_TYPE_MIFARE_UL: {
                for (auto i = 0; i < nrec; i++) {
                    NdefRecord record = tag.getNdefMessage()[i];

                    if (record.getType() && ((strncmp((const char *)record.getType(),
                                                      BW_MIMETYPE, record.getTypeLength()) == 0))||
                            (strncmp((const char *)record.getType(),
                                     BW_ALT_MIMETYPE, record.getTypeLength()) == 0)) {
                        // this is for us. Payload is a JSON string.
                        String payload = String(record.getPayload(),
                                                record.getPayloadLength());
                        JsonDocument t;
                        DeserializationError e = deserializeJson(t, payload);
                        if (e == DeserializationError::Ok) {
                            doc["payload"] = t;
                            return BWTAG_PROXY_TAG;
                        }
                        const char *p = payload.c_str();
                        log_e("deserialisation failed: %s, '%s'\n",
                              e.c_str(), p ? p : "NULL" );
                        return BWTAG_NO_MATCH;
                    }
                }
            }
            break;
        default:
            ;
    }
    return BWTAG_NO_MATCH;
}

#else
void nfc_setup(void) {}
void nfc_loop(void) {}
void nfc_poll(void) {}
bool nfc_reader_present(void) {
    return false;
}
#endif
