#include "mqtt_client.h"
#include "utils.h"
#include "watchdog_timer.h"

#include <SPI.h>

MQTT_Client::MQTT_Client(WiFiClient& wifi) : PubSubClient(wifi) {}

void MQTT_Client::connect_wifi(const char* ssid, const char* pass) {
  static uint8_t nbtry = 0;
  int status = WL_IDLE_STATUS;  // the WiFi radio's status
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    // We do not get out of this loop until connected.
    // However, if the Wifi.begin block for more than SETUP_WATCHDOG
    // then the board resets.
    PRINT("Attempting to connect to WEP network, SSID: ");
    PRINTLN(ssid);
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED) {
#ifdef USE_SERIAL
      PRINT("IP address: ");
      IPAddress ip = WiFi.localIP();
      PRINTLN(ip);
#endif
    } else {
      wdt_reset();
      PRINT(nbtry);
      PRINT(" connection error: status=");
      PRINTLN(status);
      PRINTLN("Try again in 20s...");
      delay(20000);
    }
    nbtry++;
    if (nbtry > WIFI_CONNECT_RETRY) {
      PRINTLN("Gave up connecting to Wifi!");
      PRINTLN("The board should reboot now...");
      wdt_system_reset();
    }
    wdt_reset();
  }
}

bool MQTT_Client::connect_client() {
  uint8_t nbtry = 0;
  bool ok = connected();
  if (ok) return true;
  wdt_configure(11);
  while (!ok && nbtry < MAX_CLIENT_RETRIES) {
    PRINT("Attempting MQTT connection... ");
    nbtry++;
    if (connect("DeckDoor")) {
      PRINTLN("connected");
      ok = true;
    } else {
      PRINT(nbtry);
      PRINT(" failed (rc=");
      PRINT(state());
      PRINTLN(") -> try again in 20 seconds");
      // This is BAD since we are not servicing the interrupts in the mean
      // time. However this should not happen since the MQTT timeout is
      // well above the heartbeat frequency (32s > 10s)
      // And the connection was already blocking for several seconds!
      wdt_reset();
      delay(20000);
    }
    wdt_reset();
  }
  if (nbtry == MAX_CLIENT_RETRIES) {
    PRINTLN("Give up on mqtt client connect!");
    PRINTLN("The board should reboot now...");
    wdt_system_reset();
  }
  wdt_configure(9);
  // We should never return false, since we rebooted first!
  return ok;
}

void MQTT_Client::publish_msg(const char* topic, const char* message) {
  if (connect_client()) {
    publish(topic, message);
  } else {
    PRINTLN("Error: mqtt client not connected anymore!");
  }
}
