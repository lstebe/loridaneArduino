//checks whether an incoming lora msg is a configuration for the node
bool isConfig(int index, String LRpayload) {
  if (index != -1) {
    synctime = millis();
    Serial.println("Is Config by Flag cn:");
    return true;
  }
  if (LRpayload.startsWith(UIDN)) {
    Serial.println("Is individual Config by Node UID");
    return true;
  }
  Serial.println("Is not a Config by Flag cn:");
  return false;
}

//function checks whether node is in its individual sending timeframe
bool inFrame() {
  nowtime = millis();
  if ((nowtime - synctime) % tdsize >= startframe && (nowtime - synctime) % tdsize < endframe) {
    return true;
  }
  return false;
}

// analyses the incoming lora downlink msg.
// function is stored in IRAM to improve the performance

bool IRAM_ATTR onDownlink(String LRpayload) {
  // read packet

  Serial.println();
  int PL = 3; // Search Pattern Length
  int cindex = LRpayload.indexOf("cn:"); // index of config or not
  int findex = LRpayload.indexOf("fn:"); // index of frequency
  int sindex = LRpayload.indexOf("sn:"); // index of spreadingfactor
  int tindex = LRpayload.indexOf("tn:"); // index of tx power
  int diskex = LRpayload.indexOf("td:"); // index of timedisk
  int blockex = LRpayload.indexOf("BL:"); // index of sending block on or off
  int syncindex = LRpayload.indexOf("sync"); // index of syncmsg
  int sendintindex = LRpayload.indexOf("iv:"); // index to set interval

  //Serial.println(findex);
  if (!isConfig(cindex, LRpayload)) {
    return false;
  }
  confirmflag = true;
  if (findex != -1) {
    String cfreq = LRpayload.substring(findex + PL, LRpayload.indexOf(";", findex));
    frequency = cfreq.toInt() * 100000;
    Serial.println(findex);
    LoRa.setFrequency(frequency);
    Serial.println(cfreq);
    Serial.println(frequency);
  }
  if (sindex != -1) {
    String cSF = LRpayload.substring(sindex + PL, LRpayload.indexOf(";", sindex));
    SF = cSF.toInt();
    LoRa.setSpreadingFactor(SF);
    Serial.println(SF);
  }
  if (tindex != -1) {
    String ctx = LRpayload.substring(tindex + PL, LRpayload.indexOf(";", tindex));
    txPower = ctx.toInt();
    LoRa.setTxPower(txPower);
    Serial.println(txPower);
  }
  if (diskex != -1) { //when the msg contains td:<number>;<number; eg. td:3;1250; the node owns timeframe 3 in 1250ms (750 - 1000ms)
    String tdcountstr = LRpayload.substring(diskex + PL, LRpayload.indexOf(";", diskex));
    int newdiskex = LRpayload.indexOf(";", diskex) + 1;
    String tdsizestr = LRpayload.substring(newdiskex, LRpayload.indexOf(";", newdiskex));
    unsigned tdcount = tdcountstr.toInt();
    tdsize = tdsizestr.toInt();
    int duration = ceil((8 + ((400 - 4 * SF + 28) / (4 * SF) * 5)) * pow(2, SF) / (1e5));
    startframe = duration * tdcount;
    endframe = duration * (tdcount + 1) - 51;
    Serial.print("New Timewindow: ");
    Serial.print(tdcountstr);
    Serial.print(" at ");
    Serial.print(startframe);
    Serial.print(" ms of ");
    Serial.println(tdsizestr);
  }
  if (syncindex != -1) { // when a "sync" msg or a global node configuration (cn:) is received, the last synctime is the current systemtime
    synctime = millis();
    confirmflag = false;
  }
  if (sendintindex != -1) { // when a msg cn:iv:<numInMS> or <NodeUID>iv:<numInMS> the node send interval is set to this value
    String sendintervalstr = LRpayload.substring(sendintindex + PL, LRpayload.indexOf(";", sendintindex + PL));
    sendinterval = sendintervalstr.toInt();
    Serial.print("Set interval to: ");
    Serial.println(sendinterval);
  }
  if (blockex != -1) {
    if (LRpayload.substring(blockex + PL, blockex + PL + 1) == "0") {
      sendBlock = false;
    } else if (LRpayload.substring(blockex + PL, blockex + PL + 1) == "1") {
      sendBlock = true;
    }
  }

  return true;
}

// checks if the node sending interval is over AND it is in its timeframe
bool maysend(unsigned long nowtime) {
  if (nowtime - lastTimeSend >= sendinterval && inFrame() && !sendBlock) {
    return true;
  }
  return false;
}

//sets the Lora transceiver in sending mode
void txMode() {
  LoRa.disableInvertIQ();
  LoRa.idle();
}

//sets the Lora transceiver in receiving mode
void rxMode() {
  LoRa.enableInvertIQ();
  LoRa.receive();
}

void sendUplink(String msg, bool ciph) {
  txMode();
  Serial.print("Sending packet: ");
  Serial.println(counter);
#ifdef ENCRYPT
  if (ciph == true) {
    String plainstring = UIDN + msg;
    String cipherstring = cipher->encryptString(plainstring);
    String B64 = base64::encode(cipherstring);
    int i = strlen(B64.c_str());
    snprintf(ciphertext, i, "%s", B64.c_str() );
    Serial.println(ciphertext);
  } else {
    snprintf(ciphertext, 250, "%s%s", UIDN.c_str(), msg.c_str()); //print payload to a byte buffer
  }
#else
  snprintf(ciphertext, 250, "%s%s", UIDN.c_str(), msg.c_str()); // in this case ciphertext is unencrypted
#endif

#ifdef DEBUG
  Serial.println("Sending Packet: ");
  Serial.print(counter);
  Serial.print(" ");
  Serial.print(frequency);
  Serial.print(" MHz @ SF");
  Serial.println(SF);
  Serial.println(ciphertext);
#endif
  //Send LoRa packet to receiver
  unsigned long int nowtime = millis();
  if (!confirmflag) {
    lastTimeSend = millis();
  }

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(ciphertext);
  LoRa.endPacket();
  rxMode();
  counter++;
  if (counter >= 900) {
    counter = 0;
  }
}

//holdfunction that not only delays and yields but checks the lora transceiver for incoming mgs
void hold(int dur) {
  unsigned long int start = millis();
  int packetSize = 0;
  while (millis() - start < dur) {
    int packetSize = LoRa.parsePacket();
    delay(500);
  }
}

//function that will perform a bandscan for the gateway as long the node has no
//binding to one if FREQFIX is undefined. else the freq stays the same for all acknowledgements
bool acknowledge() {
  if (!binded) {
    frequency = 863e6;
    Serial.println("Set Freq to 863");
    LoRa.setFrequency(frequency);
  }
  while (!binded) {
    Serial.println("Waiting for Acknowledgement");
    int packetSize = LoRa.parsePacket();
    // try to parse LoRa packet. if not empty call the onUplink() in receive.h
    if (packetSize) {
      String LRpayload = "";
      while (LoRa.available()) {
        LRpayload = LoRa.readString();
        Serial.println(LRpayload);
      }
      if (onDownlink(LRpayload)) {
        Serial.println("Acknowledged");
        binded = true;
      }
    } else {
      LoRa.setFrequency(frequency);
      LoRa.setSpreadingFactor(SF);
#ifdef ENCRYPT
      sendUplink(UIDN, true);
#else
      sendUplink(UIDN, false);
#endif
#ifndef FREQFIX
      frequency += 2e5;
      if (frequency > 870e6) {
        SF += 1;
        if (SF > 12) {
          SF = 7;
        }
        frequency = 863e6;
      }
#endif
      delay(3500);
      //hold(3000);
    }
  }
}
