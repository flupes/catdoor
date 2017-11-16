#include "mqtt_client.h"
#include "watchdog_timer.h"

#include <SPI.h>

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
    PRINT("Attempting MQTT connection... ");
    nbtry++;
    if (connect("MistyDoor")) {
      PRINTLN("connected");
      ok = true;
    } else {
      PRINT(nbtry);
      PRINT(" failed (rc=");
      PRINT(state());
      PRINTLN(") -> try again in 20 seconds");
      // This is BAD since we are not servicing the interrupts in the mean
      // time. However this should not happen since the MQTT timeout is
      // well above the heartbeat frequency (92s > 10s)
      // And the connection was already blocking for serveral seconds!
      wdt_reset();
      delay(20000);
    }
    wdt_reset();
  }
  if (nbtry == MAX_CLIENT_RETRIES) {
    PRINTLN("Give up on mqtt client connect!");
    PRINTLN("The board should reboot in a little bit");
    wdt_system_reset();
  }
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
