# loridaneArduino
Software for Installation and Flashing of ESP32 LoRa Boards

This Software is the compliant part to the Loridane Software under setupLoridane.
It is tested on ESP32/SX1267 Lora Boards like the
- Heltec Wireless Stick
- it should work with the whole board family
- TTGO Lilygo OLED

Setup: First of all please refer to my Thesis (see setupLoridane)


Needed Libs are:

ESP32 (Espressif)
LoRa (Sandeep Mistry)/(Gateway and Node)

PubSubClient (Nick O´Leary) (Gateway)

Encryption files are included into the Sketches, credits to Jan Joseph Pal.


Before Flashing a Node:

define AES Key in credentials.h

set the same Key in your Loridane Server Instance on a RPi4 under /home/<USER>/LORIDANE/config/loridaneConfig.json
  
restart serverinstance or reload config via GUI/System
  


Before Flashing a Gateway:
  
define WiFi credentials and MQTT Credentials according to the settings of you local server setup in credentials.h

At First Use (first acknowledgement) any Node and Gateway must be Admitted in the Loridane GUI!
