bool sendDownlink(String payload) {
#ifdef DEBUG
  Serial.println();
#endif
  LoRa.beginPacket();
  LoRa.print(payload); //redirect the MQTT received data to Lora-Transceiver as char array directly without any action
  LoRa.endPacket();
#ifdef DEBUG
  Serial.println("LoRaSent");
#endif
  return true;
}
