String UIDGWdash = "GW" + WiFi.macAddress();
String UIDGW = "";
char subTopic[30];
int mqport = 1883;

//Lora Config
//Board Specific-------------
//define pins used by transceiver
#define ss 18
#define rst 14
#define dio0 26
//---------------------------
int frequency = 867e6; // 863e6 to 870e6
String cfreq;
int txPower = 20; //0-20 dB
String ctx; // "tx power as chars"
int flag = 0;
bool block_retry = false;

//individual Configurations is only to be set in this scope!
// If Serial Debugging is not wanted uncomment the definition below!
//#define DEBUG
#define PERSIST

/*
*if JSON_OUTPUT is NOT defined(-> commented out), the output will be semicolon separated, the Server understands both
*JSON output is more readable and recommended, semicolon output is a little tinier
*/
#define JSON_OUTPUT
