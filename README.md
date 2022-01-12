# loridaneArduino
Software for Installation and Flashing of ESP32 LoRa Boards

This Software is the compliant part to the Loridane Software under setupLoridane.
It is tested on ESP32/SX1276 Lora Boards like the
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
  
Every Node in Loridane has an initial Sending interval of 10minutes. So you will not get any message from any node until then, except the acknowledgement and confirmations for configuration messages. However, if you admitted the node manually under the UI Tab "Admission" you can easily set up an interval of you choice via the UI Tab "Configure Node". The chosen Interval for every Node will be persisted on the server and the Node is set to this interval automatically by the server everytime it acknowledges.


