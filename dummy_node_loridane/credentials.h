#define ENCRYPT
//#define DEBUG
#define FREQFIX

#ifdef ENCRYPT
//Encryption Settings
// AES Encryption Key
char * aes_key = "abcdefghijklmnop";
#endif

String UIDNdash = "NO" + WiFi.macAddress();
String UIDN = "";
//define the pins used by the transceiver module
#define ss 18
#define rst 14
#define dio0 26

int SF = 7; //Spreading Factor
int txPower = 20; //0-20 dB
unsigned long int frequency = 867e6;
unsigned long int startframe = 0;
unsigned long int endframe = 1000;
