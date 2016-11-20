#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#include "Adafruit_VCNL4010.h"

Ticker tickerHeartbeat;
Ticker tickerUpdate;

Adafruit_VCNL4010 proxim_sensor;

#define NB_PROXIM_SAMPLES 20
#define NB_ANOMALY_COUNTS 8

uint16_t proxim_nominal = 2000;
uint16_t proxim_anomaly = 100;
uint16_t event_count = 0;

uint16_t proxim_samples[NB_PROXIM_SAMPLES];
bool event_detected = false;

void heartbeat() {
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
    //      Serial.print("Consider positive event after ");
    //      Serial.print(NB_ANOMALY_COUNTS);
    //      Serial.print(" samples higher than nominal ");
    //      Serial.print(proxim_nominal);
    //      Serial.print(" by ");
    //      Serial.println(proxim_anomaly);
    //      Serial.print("Last proximity reading is: ");
    //      Serial.println(proxim_value);
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
  // take a sample for the average every second (update is called at 10Hz)
  if ( counter >= 9 ) {
    counter = 0;
  }
}

void door_opened() {
  Serial.println("Cat door opened");
}

void door_closed() {
  Serial.println("Cat door closed.");
}

void setup() {
  for (uint16_t i = 0; i < NB_PROXIM_SAMPLES; i++) {
    proxim_samples[i] = proxim_nominal;
  }
  Serial.begin(9600);
  if ( !proxim_sensor.begin() ) {
    Serial.println("Proximity Sensor not found: stop!");
    while (1);
  }
  else {
    Serial.println("Connected to Proximity Sensor.");
  }

  tickerHeartbeat.attach(10, heartbeat);
  tickerUpdate.attach_ms(100, update);

}

void loop() {
  static bool raised = false;
  if ( !raised && event_count >= NB_ANOMALY_COUNTS ) {
    door_opened();
    raised = true;
  }
  if ( raised && event_count == 0 ) {
    door_closed();
    raised = false;
  }
  yield();
}

