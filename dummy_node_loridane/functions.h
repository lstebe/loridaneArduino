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
//------------------------------------------------------------------------------------------------
bool inFrame() {
  nowtime = millis();
  if ((nowtime - synctime) % tdsize >= startframe && (nowtime - synctime) % tdsize < endframe) {
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
bool IRAM_ATTR onDownlink(String LRpayload) {
  // read packet
  String msghandle;

  msghandle = LRpayload;
  Serial.println();

  int PL = 3; // Search Pattern Length
  int cindex = msghandle.indexOf("cn:"); // index of config or not
  int findex = msghandle.indexOf("fn:"); // index of frequency
  int sindex = msghandle.indexOf("sn:"); // index of spreadingfactor
  int tindex = msghandle.indexOf("tn:"); // index of tx power
  int diskex = msghandle.indexOf("td:"); // index of timedisk
  int syncindex = msghandle.indexOf("sync"); // index of syncmsg
  int sendintindex = msghandle.indexOf("iv:"); // index to set interval

  //Serial.println(findex);
  if (!isConfig(cindex, msghandle)) {
    return false;
  }
  confirmflag = true;
  if (findex != -1) {
    String cfreq = msghandle.substring(findex + PL, msghandle.indexOf(";", findex));
    frequency = cfreq.toInt() * 100000;
    Serial.println(findex);
    LoRa.setFrequency(frequency);
    Serial.println(cfreq);
    Serial.println(frequency);
  }
  if (sindex != -1) {
    String cSF = msghandle.substring(sindex + PL, msghandle.indexOf(";", sindex));
    SF = cSF.toInt();
    LoRa.setSpreadingFactor(SF);
    Serial.println(SF);
  }
  if (tindex != -1) {
    String ctx = msghandle.substring(tindex + PL, msghandle.indexOf(";", tindex));
    txPower = ctx.toInt();
    LoRa.setTxPower(txPower);
    Serial.println(txPower);
  }
  if (diskex != -1) { //when the msg contains td:<number>;<number; eg. td:3;1250; the node owns timeframe 3 in 1250ms (750 - 1000ms)
    String tdcountstr = msghandle.substring(diskex + PL, msghandle.indexOf(";", diskex));
    int newdiskex = msghandle.indexOf(";", diskex) + 1;
    String tdsizestr = msghandle.substring(newdiskex, msghandle.indexOf(";", newdiskex));
    unsigned tdcount = tdcountstr.toInt();
    tdsize = tdsizestr.toInt();
    int duration = ceil((8 + ((400 - 4 * SF + 28) / (4 * SF) * 5)) * pow(2, SF) / (1e2));
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
    String sendintervalstr = msghandle.substring(sendintindex + PL, msghandle.indexOf(";", sendintindex + PL));
    sendinterval = sendintervalstr.toInt();
    Serial.print("Set interval to: ");
    Serial.println(sendinterval);
  }

  return true;
}
//------------------------------------------------------------------------------------------------
bool maysend(unsigned long nowtime) {
  if (nowtime - lastTimeSend >= sendinterval && inFrame()) {
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------------------------
void txMode() {
  LoRa.disableInvertIQ();
  LoRa.idle();
}
//------------------------------------------------------------------------------------------------
void rxMode() {
  LoRa.enableInvertIQ();
  LoRa.receive();
}
//------------------------------------------------------------------------------------------------
String sensorRead() {
  float reading1 = random(0, 36);
  float reading2 = random(0, 50000);
  char buf[50];
  snprintf(buf, 50,"%.2f%s%.2f", reading1, ";", reading2);
  Serial.println(buf);
  return (String) buf;
}
//------------------------------------------------------------------------------------------------
void sendUplink(String msg, bool ciph) {
  txMode();
#ifdef ENCRYPT
  if (ciph == true) {
    String plainstring = UIDN+msg;
    String cipherstring=cipher->encryptString(plainstring);
    String B64 = base64::encode(cipherstring);
    int i = strlen(B64.c_str());
    snprintf(ciphertext, i, "%s", B64.c_str() );
    Serial.println(ciphertext);
  } else {
    snprintf(ciphertext, 250, "%s%s", UIDN.c_str(), msg.c_str());
  }
#else
  snprintf(ciphertext, 250, "%s%s", UIDN.c_str(), msg.c_str());
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
  LoRa.beginPacket();
  LoRa.print(ciphertext);
  LoRa.endPacket();
  rxMode();
  counter++;
  if (counter >= 900) {
    counter = 0;
  }
}
//------------------------------------------------------------------------------------------------
void hold(int dur) {
  unsigned long int start = millis();
  int packetSize = 0;
  while (millis() - start < dur) {
    yield();
  }
}
//------------------------------------------------------------------------------------------------
bool acknowledge() {
#ifndef FREQFIX
  if (!binded) {
    frequency = 863e6;
    Serial.println("Set Freq to 863");
    LoRa.setFrequency(frequency);
  }
#endif
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
        sendUplink("+", false);
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
      delay(2500);
      //hold(3000);
    }
  }
}
