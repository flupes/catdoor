#ifndef _NET_CFG_H_
#define _NET_CFG_H_

#define MQTT_KEEP_ALIVE 92
#define MQTT_SOCKET_TIMEOUT 92

#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi101.h>

#include "utils.h"

#define MAX_CLIENT_RETRIES 5

// The ESP was nice since we could store the configuration in EEPROM...
// Alas, just put our secrets here!

const char* MY_WIFI_SSID = "*****";
const char* MY_WIFI_PASS = "*****";
const char* MQTT_SERVER = "172.16.0.11";
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_HEARTBEAT = "/catdoor2/heartbeat";
const char* TOPIC_MESSAGE = "/catdoor2/message";
const char* TOPIC_SOLENOIDS = "/catdoor2/solenoids";
const char* TOPIC_PROXIMITY = "/catdoor2/proximity";
const char* TOPIC_DOOR = "/catdoor2/doorstate";

void connect_wifi(WiFiClient& wifi_client, LedCtrl& status_led) {
  static uint8_t counter = 0;
  int status = WL_IDLE_STATUS;  // the WiFi radio's status
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    if (counter == 200) {
      PRINT("Attempting to connect to WEP network, SSID: ");
      PRINTLN(MY_WIFI_SSID);
      status = WiFi.begin(MY_WIFI_SSID, MY_WIFI_PASS);
      counter = 0;
    }
    counter++;
    status_led.update();
    delay(50);
  }
#ifdef USE_SERIAL
  PRINT("IP address: ");
  IPAddress ip = WiFi.localIP();
  PRINTLN(ip);
#endif
}

class MQTT_Client : public PubSubClient {
 public:
  MQTT_Client(WiFiClient& wifi) : PubSubClient(wifi){};

  bool connect_client() {
    uint8_t nbtry = 0;
    bool ok = connected();
    while (!ok && nbtry < MAX_CLIENT_RETRIES) {
      Serial.print("Attempting MQTT connection...");
      nbtry++;
      if (connect("MistyDoor")) {
        PRINTLN("connected");
        ok = true;
      } else {
        Serial.print("failed, rc=");
        Serial.print(state());
        PRINTLN(" try again in 6 seconds");
        // Wait 6 seconds before retrying
        delay(6000);
      }
    }
    if (nbtry == MAX_CLIENT_RETRIES) {
      PRINTLN("Give up on mqtt client connect!");
      PRINTLN("It would be a good time to reboot the board now...");
      abort();
    }
    return ok;
  }

  void publish_heartbeat(DateTime dt, bool daytime) {
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

  void sync_time(DateTime dt, unsigned long ms) {
    synched_time = dt;
    synched_ms = ms;
  }

  void publish_timed_msg(unsigned long ms, const char* topic,
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

 protected:
  char payload[48];
  DateTime synched_time;
  unsigned long synched_ms;
};

#endif
