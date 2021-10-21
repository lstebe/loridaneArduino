int wifitry = 0; //counter
int mqtry = 0;
unsigned long int wifitrytime = 600000; //interval for Wifi reconnect tries

bool isConfig(unsigned int index) {
  if (index != -1) {
#ifdef DEBUG
    Serial.println("Is Config");
#endif
    return true;
  }
#ifdef DEBUG
  Serial.println("Is not a Config");
#endif
  return false;
}

bool sensorSetup() {
  return true;
}

float reading() {
  return random(-100, 100);
}
void txMode() {
  LoRa.enableInvertIQ();
  LoRa.idle();
}
void rxMode() {
  LoRa.disableInvertIQ();
  LoRa.receive();
}
//#------------------------------------------------BEGIN SETUP WIFI connecrtions--------------------------------------------------------------------------------
void wifiSetup(char* sd, char* pwd) {
  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);
  if (block_retry == false) {
    return;
  }

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", sd);
  //IPAddress ip(192,168,2,160); //if IP settings should be static, this can be done here
  //IPAddress gateway(192,168,2,1);
  //IPAddress subnet(255,255,255,0);
  WiFi.persistent(false); // >false< -> less duty cycles on flash memmory when using non persistent credentials when they are hardcoded and not changed here anyway
  //WiFi.config(ip,gateway,subnet);
  WiFi.begin(sd, pwd);
  // Wait for Connection
  while ( WiFi.status() != WL_CONNECTED && wifitry <= 50) { //WiFi.status()
#ifdef DEBUG
    Serial.print(".");
#endif
    wifitry++;
    delay(200);
  }
  Serial.println();
  wifitry  = 0;
  wifitrytime = millis();
  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}
//#-----------------------------------------------END SETUP WIFI connections---------------------------------------------------------------------------------

//#----------------------------------------------BEGIN SETUP MQTT connections----------------------------------------------------------------------------------
void MQreconnect( char* musr, char* mpwd) {
  if (block_retry == false) {
    return;
  }
  // Loop until MQTT Broker (re)connected
  while (!client.connected() && mqtry <= 5) {
    Serial.print("Attempting MQTT connection...");
    randomSeed(analogRead(36)); // noise on analog port as seed for the random algorithm, therefore it is not initialised as Input
    String clientId = "ESP32LORA-";
    clientId += String(random(0xffff), HEX); // Create a random client ID in HEX
    // Attempt to connect
    if (client.connect(clientId.c_str(), musr, mpwd)) {
      Serial.println("connected");
      client.publish("lora/gateway/", UIDGW.c_str()); //publish an announcement when connected
    } else {
      Serial.print("failed, reconnect=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      mqtry++;
      // Wait 5 seconds before retrying
      delay(3000);
    }
    if (mqtry == 5) {
      Serial.println("Connection not establishable");
      Serial.println("Please type <mqtt>, no Brackets, in the Serial Input and set new MQTT Credentials");
    }
  }
  snprintf (subTopic, 30, "lora/%s/#", UIDGW.c_str()); // subscribe topic e.g. "lora/dcxd34b4/# the MQTT channel for the gateway
  Serial.println("Subscribing to topic:");
  Serial.println(subTopic);
  client.subscribe((const char*) subTopic);
  mqtry = 0;
}
//#------------------------------------------------END SETUP MQTT Connections--------------------------------------------------------------------------------
//#------------------------------------------------Payload Encryption--------------------------------------------------------------------------------
#ifdef ENCRYPT
void aes_init() {
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);
}

uint16_t encrypt_to_ciphertext(char * msg, uint16_t msgLen, byte iv[]) {
#ifdef DEBUG
  Serial.println("encrypt...");
#endif
  // aesLib.get_cipher64_length(msgLen);
  int cipherlength = aesLib.encrypt((byte*)msg, msgLen, (char*)ciphertext, aes_key, sizeof(aes_key), iv);
  // uint16_t encrypt(byte input[], uint16_t input_length, char * output, byte key[],int bits, byte my_iv[]);
  return cipherlength;
}

uint16_t decrypt_to_cleartext(byte msg[], uint16_t msgLen, byte iv[]) {
  uint16_t dec_bytes = aesLib.decrypt(msg, msgLen, (char*)cleartext, aes_key, sizeof(aes_key), iv);
#ifdef DEBUG
  Serial.print("decrypt...; ");
  Serial.print("Decrypted bytes: "); Serial.println(dec_bytes);
#endif
  return dec_bytes;
}
#endif
