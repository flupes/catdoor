#include <EEPROM.h>

const uint8_t MAX_STRING_LENGTH = 32;

const char *SSID = "MySuperWifi-2g";
const char *PASS = "very$ecretPass";

void str2eeprom(uint16_t addr, const char *str) {
  uint8_t len = strlen(str);
  for (uint8_t c = 0; c<MAX_STRING_LENGTH; c++) {
      if ( c < len ) {
        EEPROM.write(addr+c, str[c]);
      }
      else {
        EEPROM.write(addr+c, 0);
      }
  }
  EEPROM.commit();
}

void setup() {
  EEPROM.begin(MAX_STRING_LENGTH*2);
  str2eeprom(0, SSID);
  str2eeprom(MAX_STRING_LENGTH, PASS);
}

void loop() {
}
