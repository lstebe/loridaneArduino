/*****
   This Software provides the functionality of redirecting LoRa messages to an MQTT broker on LoRa equiped ESP32 chips and was tested with "Heltec Wireless Stick (Lite)" and TTGO Lilygo Lora Boards
   (as purchased in May 2021) with Semtech SX1276 LoRa transceivers
   From the triplet of SERVER, GATEWAY and NODE this is the NODE Software
   NODE here refers to an Arduino compatible chipset of the same Architecture as mentioned above
   SERVER here refers to a Raspberry Pi with a NODE-RED instance and an MQTT Broker compatible with MQTT v3.1.1, but can be any Broker/Client that is able to send, receive an process MQTT messages

   This code is part of the Bachelor-Thesis at
   <ETA, PTW Institute, TU Darmstadt, Germany>
   Title: <Evaluation of a low-cost energy metering concept implemented as a LoRa network for energy key performance indicator aggregation in a production environment>
   written and maintained by
   Author:  <Lindsay Stebe>
   EMail:   <Lindsay@Lstebe.de>
   Year:    <2021>
   and is accessible under GNU GPL v3
   For reusing this code including the project files (credentials.h, functions.h, receive.h, send.h) either "as is" or in parts permission is hereby granted.
   NOT permitted is selling and relicencing this software, neither as whole, nor in parts, nor on preconfigured hardware.
   This header may not be edited or removed, except by the author
   The author provides this software as is without any warranty of usability or functionality. The responsibility of using this software in terms of keeping notice of hardware restrictions and
   local Law lies by the USER - respectivly everyone who executes this code.
   Used Libraries provided by individuals: <Sandeep Mistry, LoRa> <Jan Joseph Pal, cipher.cpp, encrypt.h>
   ----------------------------------------------------------------------Version: <2021_11_20>--------------------------------------------------------------------------------------
*/


#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include "credentials.h"
#include <base64.h>
#ifdef ENCRYPT
#include "encrypt.h"
Cipher * cipher = new Cipher();
#endif

#define INPUT_BUFFER_LIMIT (250 + 1)
char ciphertext[2 * INPUT_BUFFER_LIMIT] = {0}; //encrypted
bool confirmflag = false;
bool binded = false;
int counter = 0;
int tdcount = 0;
long int tdsize = 1000;
long int tdsegmentstart = 0;
unsigned long int synctime = 0;
unsigned long int nowtime = millis();
unsigned long int lastTimeSend = 0;
long int sendinterval = 600000;
String payload;
int packetSize;
WiFiClient espClient;
#include "functions.h"

void setup() {
  for (int i = 0; i < UIDNdash.length(); i++) {
    if (UIDNdash.substring(i, i + 1) != ":") {
      UIDN.concat(UIDNdash[i]);
    }
  }
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
  WiFi.mode(WIFI_OFF);
  btStop();
#ifdef ENCRYPT
  cipher->setKey(aes_key);
#endif
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(frequency)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(SF);
  LoRa.setTxPower(txPower);
  Serial.println("LoRa Initializing OK!");
#ifdef ENCRYPT
  sendUplink(UIDN, false);
#else
  sendUplink(UIDN, false);
#endif
  //hold(3000);
  delay(3500);
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LRpayload = "";
    while (LoRa.available()) {
      LRpayload = LoRa.readString();
      Serial.println(LRpayload);
    }
    if (onDownlink(LRpayload)) {
      Serial.println("Acknowledged");
      //sendUplink((String)sensorRead());
      binded = true;
    }
  } else {
    acknowledge();
  }

}

void loop() {
  // try to parse packet
  nowtime = millis();

  int packetSize = LoRa.parsePacket();
  // try to parse LoRa packet. if not empty call the onUplink() in receive.h
  if (packetSize) {
    String LRpayload = "";
    while (LoRa.available()) {
      LRpayload = LoRa.readString();
      Serial.println(LRpayload);
    }
    onDownlink(LRpayload);
  }
  if (confirmflag && inFrame()) {
    sendUplink("+", false);
    confirmflag = false;

  }

  if (maysend(nowtime)) {
#ifdef ENCRYPT
    //sendUplink((String) sensorRead(), true);
    String reading = sensorRead();
    while ((reading.length() + 14) % 16 != 0) {
      reading += '\0';
    }
    sendUplink(reading, true);
    Serial.println(sensorRead());
#else
    sendUplink((String) sensorRead(), false);
#endif
  }
}
