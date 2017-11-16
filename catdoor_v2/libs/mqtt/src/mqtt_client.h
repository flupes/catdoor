#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_

// Definition for MQTT configuration comes BEFORE including
// the mqtt library header so they override correctly the defaults
#define MQTT_KEEP_ALIVE 90
#define MQTT_SOCKET_TIMEOUT 6

#include <PubSubClient.h>
#include <WiFi101.h>

#include "RTClib.h"
#include "VisualFeedback.h"
#include "ledctrl.h"
#include "utils.h"

// Topic names definition
#define TOPIC_HEARTBEAT "/catdoor/heartbeat"
#define TOPIC_MESSAGE "/catdoor/message"
#define TOPIC_SOLENOIDS "/catdoor/solenoids"
#define TOPIC_PROXIMITY "/catdoor/proximity"
#define TOPIC_DOORSTATE "/catdoor/doorstate"
#define TOPIC_BATTERY_V "/catdoor/battery_v"

#define MAX_CLIENT_RETRIES 6

class MQTT_Client : public PubSubClient {
 public:
  MQTT_Client(WiFiClient& wifi, VisualFeedback& fb)
      : PubSubClient(wifi), visual(fb){};

  void connect_wifi(const char* ssid, const char* pass);

  bool connect_client();

  void publish_heartbeat(DateTime dt, bool daytime);

  void publish_timed_msg(unsigned long ms, const char* topic,
                         const char* message);

  void sync_time(DateTime dt, unsigned long ms) {
    synched_time = dt;
    synched_ms = ms;
  }

 protected:
  char payload[48];
  DateTime synched_time;
  unsigned long synched_ms;
  VisualFeedback& visual;
};

#endif
