/* Copyright_License {

  Copyright (C) 2006-2022 www.hlballon.com
  Version 1.0 220701 Hilmar Lorenz
  Version 1.1 220722 Hilmar "readingId add"




  The fundamentals of our implementation for a HotAirBallooning Burner Control System (HLBc) are described on

  http://www.hlballon.com/brennersteuerung.php


  The system realizes the barometric acceleration measurement and provides a direct estimation of the
  vertical forces applied by external impulses (burner, meteorological influences) to the balloon.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/
#include <esp_now.h>
#include <WiFi.h>
#include <M5Unified.h>
#include "espnow.h"


// REPLACE WITH YOUR RECEIVER MAC Address
// Core2 Control 1
// uint8_t broadcastAddress[] = {0x08, 0x3A, 0xF2, 0x43, 0x59, 0x9C};
// Core2 Control 2
uint8_t broadcastAddress[] = {0x30, 0xC6, 0xF7, 0x15, 0x30, 0xD4};

unsigned int readingId = 0;

// Structure example to send data
// Must match the receiver structure
/*
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;
*/
typedef struct struct_message {
    int id;
    float p;
    float v;
    float acc;
    float vA;
    float vV;
    int readingId;
} struct_message;

// Create a struct_message called slvDat
struct_message slvDat;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

    // Uncomment for Testimg!!!!

    //  Serial.print("\r\nLast Packet Send Status:\t");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void espnow_setup() {

    // Serial.begin(115200);
    // Serial.begin(9600);

    WiFi.mode(WIFI_MODE_STA);
    Serial.println(WiFi.macAddress());

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5,50);
    M5.Lcd.print(WiFi.macAddress());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void espnow_send(bool ok, float pressure,
                 float v_baro, float a_baro,
                 double altVar, double varioVar) {
    // Set values to send
    slvDat.id = 1;
    slvDat.v = pressure;
    slvDat.v = v_baro;
    slvDat.acc = a_baro;
    slvDat.vA = altVar;
    slvDat.vV = varioVar;
    slvDat.readingId = readingId++;


    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &slvDat, sizeof(slvDat));


    // Uncomment for Testing!!!!!
    /*
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
    */

}

