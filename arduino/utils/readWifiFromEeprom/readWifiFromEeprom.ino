#include <EEPROM.h>

const uint8_t MAX_STRING_LENGTH = 32;

void eeprom2str(uint16_t addr, char *str) {
  for (uint8_t c = 0; c < MAX_STRING_LENGTH; c++) {
    str[c] = EEPROM.read(addr + c);
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(MAX_STRING_LENGTH * 2);
  char ssid[MAX_STRING_LENGTH];
  char pass[MAX_STRING_LENGTH];
  eeprom2str(0, ssid);
  eeprom2str(MAX_STRING_LENGTH, pass);
  Serial.println("Got Wifi Configuration:");
  Serial.print("SSID = ");
  Serial.println(ssid);
  Serial.print("password = ");
  Serial.println(pass);
}

void loop() {
}

