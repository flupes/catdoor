#include "mqtt_client.h"
#include "watchdog_timer.h"

#include <SPI.h>

void MQTT_Client::connect_wifi(const char* ssid, const char* pass) {
  static uint8_t nbtry = 0;
  int status = WL_IDLE_STATUS;  // the WiFi radio's status
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    // We do not get out of this loop until connected.
    // However, if the Wifi.begin block for more than SETUP_WATCHDOG
    // then the board resets.
    // In addition, after 5 minutes of trying (15 try x 20s), we also
    // force a reset. Reset will trigger unjam, we do not want too
    // unjamming too often when network is totally down (5 minutes is good).
    visual.warning();
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
      visual.error();
      wdt_reset();
      PRINT(nbtry);
      PRINT(" connection error: status=");
      PRINTLN(status);
      PRINTLN("Try again in 20s...");
      delay(20000);
    }
    nbtry++;
    if (nbtry > 15) {
      PRINTLN("Gave up connecting to Wifi!");
      PRINTLN("The board should reboot now...");
      wdt_system_reset();
    }
    wdt_reset();
  }
  visual.ok();
}

bool MQTT_Client::connect_client() {
  uint8_t nbtry = 0;
  bool ok = connected();
  if (ok) return true;
  // Here we rely on the the quality of the Wifi101 + MQTT connect
  // We disable the WDT, hoping that they do the right stuff and return
  // at one point... Otherwise we will never reset!
  wdt_configure(11);
  // wdt_disable();
  while (!ok && nbtry < MAX_CLIENT_RETRIES) {
    visual.warning();
    PRINT("Attempting MQTT connection... ");
    nbtry++;
    if (connect("MistyDoor")) {
      PRINTLN("connected");
      ok = true;
    } else {
      visual.error();
      PRINT(nbtry);
      PRINT(" failed (rc=");
      PRINT(state());
      PRINTLN(") -> try again in 20 seconds");
      // This is BAD since we are not servicing the interrupts in the mean
      // time. However this should not happen since the MQTT timeout is
      // well above the heartbeat frequency (92s > 10s)
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
  visual.ok();
  wdt_configure(4);
  // We should never return false, since we rebooted first!
  return ok;
}

void MQTT_Client::publish_heartbeat(DateTime dt, bool daytime) {
  dt.toString(payload);
  size_t len = strlen(payload);
  if (daytime) {
    strcpy(payload + len, " OPEN");
  } else {
    strcpy(payload + len, " LOCKED");
  }
  PRINT("payload=");
  PRINTLN(payload);
  if (connect_client()) {
    publish(TOPIC_HEARTBEAT, payload);
  } else {
    PRINTLN("Error: mqttClient not connected anymore");
  }
}

void MQTT_Client::publish_timed_msg(unsigned long ms, const char* topic,
                                    const char* message) {
  TimeSpan duration((ms - synched_ms) / 1000);
  DateTime current = synched_time + duration;
  sprintf(payload, "%04d-%02d-%02d %02d:%02d:%02d %s",  // date time payload
          current.year(), current.month(), current.day(), current.hour(),
          current.minute(), current.second(), message);
  if (connect_client()) {
    publish(topic, payload);
  } else {
    PRINTLN("Error: mqtt client not connected anymore!");
  }
}
