#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PubSubClient.h>
// #include <Ticker.h>

#include "Adafruit_VCNL4010.h"

// Ticker tickerHeartbeat;
// Ticker tickerUpdate;

Adafruit_VCNL4010 proxim_sensor;

#define NB_PROXIM_SAMPLES 20
#define NB_ANOMALY_COUNTS 15

#define MAX_CLIENT_RETRIES 10

uint16_t proxim_nominal = 2000;
uint16_t proxim_anomaly = 100;
uint16_t event_count = 0;

uint16_t proxim_samples[NB_PROXIM_SAMPLES];
bool event_detected = false;

const uint8_t MAX_STRING_LENGTH = 32;
const char *MQTT_SERVER = "172.16.0.11";
const uint16_t MQTT_PORT = 1883;

WiFiClient espWifiClient;
PubSubClient mqttClient(espWifiClient);

void heartbeat() {
  publish_heartbeat();
  Serial.print("average promimity = ");
  Serial.println(proxim_nominal);
}

void update() {
  static uint16_t counter = 0;
  static uint16_t index = 0;
  uint16_t proxim_value = proxim_sensor.readProximity();
  if ( proxim_value > proxim_nominal + proxim_anomaly ) {
    // integrate the anomaly
    if (event_count < NB_ANOMALY_COUNTS) {
      event_count++;
    }
    //    else {
    //      Serial.print("Consider positive event after "); Serial.print(NB_ANOMALY_COUNTS);
    //      Serial.print(" samples higher than nominal "); Serial.print(proxim_nominal);
    //      Serial.print(" by "); Serial.println(proxim_anomaly);
    //      Serial.print("Last proximity reading is: "); Serial.println(proxim_value);
    //    }
  } else {
    if ( event_count > 0 ) {
      event_count--;
    }
    if ( counter == 0 ) {
      proxim_samples[index] = proxim_value;
      index++;
      if (index >= NB_PROXIM_SAMPLES - 1) {
        index = 0;
      }
      // slow recompute the average rather than dealing with wrap-around indexes
      // should be optimized to only removed the older value and add the new value
      proxim_nominal = 0;
      for (uint16_t i = 0; i < NB_PROXIM_SAMPLES; i++) {
        proxim_nominal += proxim_samples[i];
      }
      proxim_nominal /= NB_PROXIM_SAMPLES;
    }
    counter++;
  }
  // take a sample for the average every second (update is called at 20Hz)
  if ( counter >= 20 ) {
    counter = 0;
  }
}

bool connectClient() {
  uint8_t nbtry = 0;
  bool connected = mqttClient.connected();
  while ( !connected && nbtry < MAX_CLIENT_RETRIES ) {
    Serial.print("Attempting MQTT connection...");
    nbtry++;
    if (mqttClient.connect("MistyDoor")) {
      Serial.println("connected");
      connected = true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  if ( nbtry == MAX_CLIENT_RETRIES ) {
    Serial.println("Give up on mqtt client connect!");
    Serial.println("It would be a good time to reboot the board now...");
    // system_restart();
  }
  return connected;
}

void publish_heartbeat() {
  char payload[8];
  sprintf(payload, "%d", proxim_nominal);
  if ( connectClient() ) {
    mqttClient.publish("/catdoor/avgproxim", payload);
  }
  else {
    Serial.println("Error: mqttClient not connected anymore");
  }
}

void publish_state(int8_t state) {
  char payload[8];
  if (state) {
    strcpy(payload, "open");
  }
  else {
    strcpy(payload, "closed");
  }
  if ( connectClient() ) {
    mqttClient.publish("/catdoor/state", payload);
  }
  else {
    Serial.println("Error: mqttClient not connected anymore");
  }
}

void door_opened() {
  Serial.println("Cat door opened");
  publish_state(1);
}

void door_closed() {
  Serial.println("Cat door closed.");
  publish_state(0);
}

void eeprom2str(uint16_t addr, char *str) {
  for (uint8_t c = 0; c < MAX_STRING_LENGTH; c++) {
    str[c] = EEPROM.read(addr + c);
  }
}

void setup_wifi() {
  // Get SSID and password from EEPROM
  EEPROM.begin(MAX_STRING_LENGTH * 2);
  char ssid[MAX_STRING_LENGTH];
  char pass[MAX_STRING_LENGTH];
  eeprom2str(0, ssid);
  eeprom2str(MAX_STRING_LENGTH, pass);
  Serial.println();
  Serial.print("Connecting to network with SSID = ");
  Serial.println(ssid);
  delay(100);

  // Connect to Wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void setup() {
  // Sensor
  for (uint16_t i = 0; i < NB_PROXIM_SAMPLES; i++) {
    proxim_samples[i] = proxim_nominal;
  }
  Serial.begin(115200);
  if ( !proxim_sensor.begin() ) {
    Serial.println("Proximity Sensor not found: stop!");
    while (1);
  }
  else {
    Serial.println("Connected to Proximity Sensor.");
  }

  // Network
  setup_wifi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  // Schedulers
  // tickerHeartbeat.attach(10, heartbeat);
  // tickerUpdate.attach_ms(100, update);

}

void loop() {
  static bool raised = false;
  static uint8_t counter = 0;
  static unsigned long last_millis = millis();

  // publish heartbeat
  unsigned long now_millis = millis();
  if ( now_millis - last_millis > 50 ) {
    update();
    counter++;
    if ( counter == 200 ) {
      heartbeat();
      counter = 0;
    }
    last_millis = now_millis;
  }

  // take care of event if any
  if ( !raised && event_count >= NB_ANOMALY_COUNTS ) {
    door_opened();
    raised = true;
  }
  if ( raised && event_count == 0 ) {
    door_closed();
    raised = false;
  }

  // Let the MQTT client do some work
  mqttClient.loop();

  delay(10);

  // Let the ESP Wifi do some work?
  //yield();
}

