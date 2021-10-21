String UIDNdash = "NO" + WiFi.macAddress();
String UIDN = "";

//define the pins used by the transceiver module
#define ss 18
#define rst 14
#define dio0 26

int SF = 7; //Spreading Factor
int txPower = 20; //0-20 dB
unsigned long int frequency = 867e6;
