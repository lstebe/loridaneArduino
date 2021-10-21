/* ATTENTION!
    if you
    #define PERSIST
    in "init.h" you will have to provide the credentials in SPIFFS or write them there via serial input before!
*/

#include <Preferences.h>
Preferences preferences;

bool loadFlashData() {

  preferences.getString("ssid", "").toCharArray(sd, 33);
  preferences.getString("pwd", "").toCharArray(pwd, 33);
  preferences.getString("madr", "").toCharArray(madr, 16);
  preferences.getString("musr", "").toCharArray(musr, 33);
  preferences.getString("mpwd", "").toCharArray(mpwd, 33);

  return true;
}
