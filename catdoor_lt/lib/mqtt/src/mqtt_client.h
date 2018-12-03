#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_

#define WIFI_CONNECT_RETRY 10

// Definition for MQTT configuration comes BEFORE including
// the mqtt library header so they override correctly the defaults
#define MQTT_KEEP_ALIVE 90
#define MQTT_SOCKET_TIMEOUT 6

#include <PubSubClient.h>
#include <WiFi101.h>

// Topic names definition
#define TOPIC_MESSAGE "/catdoor/message"
#define TOPIC_DOORSTATE "/catdoor/state"
#define TOPIC_BATTERY_V "/deckdoor/battery_v"

#define MAX_CLIENT_RETRIES 6

class MQTT_Client : public PubSubClient {
 public:
  MQTT_Client(WiFiClient& wifi);

  void connect_wifi(const char* ssid, const char* pass);

  bool connect_client();

  void publish_msg(const char* topic, const char* message);

 protected:
  char payload_[48];
};

#endif
