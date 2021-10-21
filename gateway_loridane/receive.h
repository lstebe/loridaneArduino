String onUplink() {
  String LoRaData;

  // read packet
  String LRpayload = "";
  while (LoRa.available()) {
    LoRaData = LoRa.readString();
#ifdef DEBUG
    Serial.println(LoRaData);
#endif
    LRpayload += LoRaData;
  }
  return LRpayload;
}
