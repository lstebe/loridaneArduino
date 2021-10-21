void readSerialIn(String initiation) {
  int i;
  char msg[i] = {};

  if (initiation == "wifi") {
    flag = 1;
    Serial.println("Please Enter Your WiFi-SSID");
    return;
  } else  if (initiation == "mqtt") {
    Serial.println("Please Enter the Address Of Your MQTT Broker without the Port");
    flag = 3;
    return;
  } else {
    switch (flag) {
      case 1: // SSID
        i = (initiation.length() + 1);
        initiation.toCharArray(sd, i);
        Serial.println(sd);
        
        //Save to Flash
        preferences.begin("credentials", false);
        preferences.putString("ssid", sd);
        preferences.end();
        
        Serial.println("Please Enter Your WiFi-Password");
        flag = 2;
        block_retry = true;
        break;
      case 2: //WifiPassword
        i = (initiation.length() + 1);
        initiation.toCharArray(pwd, i);
        flag = 0;
        Serial.println(pwd);
        
        preferences.begin("credentials", false);
        preferences.putString("pwd", pwd);
        preferences.end();
        
        Serial.println("WiFi Credentials updated, try to connect.");
        WiFi.disconnect();
        wifiSetup(sd, pwd);
        break;

      case 3: //MQTT address
        i = (initiation.length() + 1);
        initiation.toCharArray(madr , i);
        Serial.println(madr);
        block_retry = true;
        
        preferences.begin("credentials", false);
        preferences.putString("madr", madr);
        preferences.end();
        
        Serial.println("Please Enter the Username Of Your MQTT Broker");
        flag = 5;
        break;
      /*
        case 4: //MQTT Port
        mqport = initiation.toInt();
        Serial.println(mqport);
        Serial.println("Please Enter the Username for Your MQTT Broker");
        flag = 5;
        break;
        // */
      case 5: //5 MQTT User
        i = (initiation.length() + 1);
        initiation.toCharArray(musr , i);
        Serial.println(musr);
        
        preferences.begin("credentials", false);
        preferences.putString("musr", musr);
        preferences.end();
        
        Serial.println("Please Enter the Password for Your MQTT Broker");
        flag = 6;
        break;
      case 6: //6 MQTT Password
        i = (initiation.length() + 1);
        initiation.toCharArray(mpwd , i);
        Serial.println(mpwd);
        Serial.println("MQTT Credentials updated");
        flag = 0;
        block_retry = false;

        preferences.begin("credentials", false);
        preferences.putString("mpwd", mpwd);
        preferences.end();

        client.setServer(madr, mqport);
        MQreconnect(musr, mpwd);
        break;
    }
    return;
  }
}
