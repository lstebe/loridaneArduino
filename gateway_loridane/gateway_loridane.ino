/*****
   This Software provides the functionality of redirecting LoRa messages to an MQTT broker on LoRa equiped ESP32 chips and was tested with "Heltec Wireless Stick (Lite)" and TTGO Lilygo Lora Boards
   (as purchased in May 2021) with Semtech SX1276 LoRa transceivers
   From the triplet of SERVER, GATEWAY and NODE this is the GATEWAY Software
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
   For reusing this code including the project files (credentials.h, functions.h, receive.h, send.h,init.h,serialinput.h) either "as is" or in parts permission is hereby granted.
   NOT permitted is selling and relicencing this software, neither as whole, nor in parts, nor on preconfigured hardware.
   This header may not be edited or removed, except by the author
   The author provides this software as is without any warranty of usability or functionality. The responsibility of using this software in terms of keeping notice of hardware restrictions and
   local Law lies by the USER - respectivly everyone who executes this code.
   Used Libraries provided by individuals: <Sandeep Mistry, LoRa> <Nick O'Leary, PubSubClient>
   ----------------------------------------------------------------------Version: <2021_11_20>--------------------------------------------------------------------------------------
*/

//#include <ArduinoOTA.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <PubSubClient.h>

WiFiClient espClient; //Object Definition for WiFi and MQTT
PubSubClient client(espClient);

#include "init.h"
#include "credentials.h"
#include "receive.h"
#include "send.h"
#include "functions.h"


#ifdef PERSIST
#include "persist.h"
#endif
#include "serialinput.h"

float SNR; //Signal to Noise Ration
String cSNR;
int SF = 7; //Spreading Factor
String cSF;
int syncWord = 0xF3; //hex preamble for syncronisation between GW and Node 0x00-0xFF TTN:0x34
String csy;
unsigned long int nowtime = millis();
String charUIDGW; //Gateway UID as char array
char charbuf[250]; //Buffer that keeps incoming LoRa Data
String LRpayload = "";
String MQpayload; //Buffer that keeps incoming MQTT Data






//#---------------------------------------------BEGIN the callback for incoming MQTT msg-----------------------------------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) { //on received MQTT Message
#ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();
#endif
  txMode(); //change LoRa to sendmode
  MQpayload = "";
  byte* p = (byte*)malloc(length); // allocate a new RAM space with 250bytes for the MQTT message
  memcpy(p, payload, length); //copy the (destructable) original message to the new space
  memset(&MQpayload[0], 0, sizeof(MQpayload)); //init the char array as empty (zero initialized) memory space
  for (int i = 0; i < length; i++) { //save payload to char[]
#ifdef DEBUG
    Serial.print((char)payload[i]);
#endif
    MQpayload += (char)payload[i];
  }
  if (MQpayload != "ping") {
    sendDownlink(MQpayload);
    rxMode(); //receivemode again
  } else {
    client.publish("lora/gateway/", "pong");
  }

  /*Example input from MQTT >> "This is a payload string which should be a forwarded cn:config, therefore it sets a new frequency fg:8690; a spreadingfactor sg:10; and the
     TX power of the node to tg:15; dBM"
     This payload is analysed in the next lines, except the Config flag cn: it requires all parameters to be initialisized with the parameter "p" and ":" and terminated by ";" -> p:<VALUE>;
     pn:<VALUE>; for nodes, pg:<VALUE>; for the gateway
  */
  int PL = 3; // Search Pattern Length
  int cindex = MQpayload.indexOf("cg:") ; // if msg contains "cg:" its a config
  int findex = MQpayload.indexOf("fg:"); // index of frequency
  int sindex = MQpayload.indexOf("sg:"); // index of spreadingfactor
  int tindex = MQpayload.indexOf("tg:"); // index of tx power
  //Serial.println(findex);
  if (!isConfig(cindex)) {
    return;
  }
  if (findex > -1) {
    cfreq = MQpayload.substring(findex + PL, MQpayload.indexOf(";", findex));
    frequency = cfreq.toInt() * 100000; //we save few bytes of sent data by deviding the frequency serverside by 1e5 and multiply it here.
#ifdef DEBUG
    Serial.println(findex);
#endif
    LoRa.setFrequency(frequency);
#ifdef DEBUG
    Serial.println(cfreq);
    Serial.println(frequency);
#endif
  }
  if (sindex > -1) {
    cSF = MQpayload.substring(sindex + PL, MQpayload.indexOf(";", sindex));
    SF = cSF.toInt();
    LoRa.setSpreadingFactor(SF);
#ifdef DEBUG
    Serial.println(SF);
#endif
  }
  if (tindex > -1) {
    ctx = MQpayload.substring(tindex + PL, MQpayload.indexOf(";", tindex));
    txPower = ctx.toInt();
    LoRa.setTxPower(txPower);
#ifdef DEBUG
    Serial.println(txPower);
#endif
  }

  free(p); //clear the earlier allocated memory on p
  //client.publish("lora/test/", "Got it!");

}
//#----------------------------------------------END the callback for incoming MQTT msg----------------------------------------------------------------------------------

//#----------------------------------------------BEGIN MQTT sender----------------------------------------------------------------------------------
void MQsend(String LRpayload) { //is a function that writes the payload and additional information to a char array pointer and converts it for sending via MQTT
  char Data[250];

  //to minimize data-usage only semicolon seperated values are used and reordered to an object serverside
  // the semicolon is randomly chosen as it will not appear in our payloads
#ifndef JSON_OUTPUT
  snprintf(Data, 250, "%s;%s;%i;%f;%i;%ld", LRpayload.c_str(), UIDGW.c_str(), LoRa.packetRssi(), LoRa.packetSnr(), SF, frequency);
#else
  snprintf(Data, 250, "{\"payload\":\"%s\",\"gw\":\"%s\",\"rssi\":%i,\"snr\":%f,\"sf\":%i,\"freq\":%ld}", LRpayload.c_str(), UIDGW.c_str(), LoRa.packetRssi(), LoRa.packetSnr(), SF, frequency);
#endif
  client.publish("lora/gateway/", Data); //send the MQTT msg that contains the payload and additional data
  //client.subscribe((const char*) "lora/testrec/#"); // resubscribe to a topic that should be listened
}
//#----------------------------------------------END MQTT sender----------------------------------------------------------------------------------

//#----------------------------------------------BEGIN ESP setup()----------------------------------------------------------------------------------
void setup() {
  //initialize Serial Monitor only for debugging purposes
  Serial.begin(115200);
  while (!Serial);
  for (int i = 0; i < UIDGWdash.length(); i++) {
    if (UIDGWdash.substring(i, i + 1) != ":") {
      UIDGW.concat(UIDGWdash[i]);
    }
  }
#ifdef DEBUG
  Serial.println("Setup LoRa Receiver");
#endif
  //WiFi.mode(WIFI_OFF);
  btStop(); //we do not need bluetooth
  /*
    //open the config namespace in flash memory
    persistentData.begin("config", true);
    UIDGW = persistentData.getString("UID", "00000000", 16); //load UID if available else "0"
    persistentData.end();
    //snprintf(charUIDGW, 8, "%s", UIDGW); //convert UID to char[]
    //ArduinoOTA.setHostname(charUIDGW); //setUID as Hostname vor Wifi Firmware Update
  */
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(frequency)) {
#ifdef DEBUG
    Serial.println(".");
#endif
    delay(100);
  }
  // Change sync word (0xF3) to match the receiver, so we won't receive packets from "foreign" nodes
  LoRa.setSyncWord(syncWord);
  LoRa.setSpreadingFactor(SF); //defaults to 7
  LoRa.setTxPower(txPower); //defaults to 20 (=max)
#ifdef DEBUG
  Serial.println("LoRa Initializing OK!");
#endif
#ifdef PERSIST
  //Try to load Data from Flash
  preferences.begin("credentials", false);
  loadFlashData();
  preferences.end();
#else
  strcpy(musr, mqtt_usr);
  strcpy(mpwd, mqtt_pwd);
  strcpy(madr, mqtt_server);
  strcpy(sd, ssid);
  strcpy(pwd, password);
#endif
  if (sd == "" || pwd == "") {
    strcpy(musr, mqtt_usr);
    strcpy(mpwd, mqtt_pwd);
    strcpy(madr, mqtt_server);
    strcpy(sd, ssid);
    strcpy(pwd, password);
  } else {
    wifiSetup(sd, pwd);
    MQreconnect(musr, mpwd);
  }
}

client.setServer(madr, mqport); //(... ,1883) for unencrypted
client.setCallback(callback);

/*if (UIDGW == "00000000") { // if no UID yet set request one from the RPi server
  client.publish("lora/0/acknowledge/", "1");
  }*/
Serial.println("Gateway UID:");
Serial.println(UIDGW);
}
//#---------------------------------------------------END ESP32 Setup-----------------------------------------------------------------------------

//#---------------------------------------------------Enter loop()-----------------------------------------------------------------------------
void loop() {
  //ArduinoOTA.handle();

  nowtime = millis();

  if (WiFi.status() != WL_CONNECTED && nowtime - wifitrytime > 600000) {
    wifiSetup(sd, pwd);
  }
  if (!client.connected() && WiFi.status() == WL_CONNECTED && block_retry == false) {
    MQreconnect(musr, mpwd);
  }
  client.loop();

  int packetSize = LoRa.parsePacket();
  // try to parse LoRa packet. if not empty call the onUplink() in receive.h
  if (packetSize) {
    String pay = onUplink();
    MQsend(pay); //redirect it to MQTT
  }
  if (Serial.available() > 0) {
    Serial.println("Serial Input");
    readSerialIn(Serial.readStringUntil('\n'));
  }
}
