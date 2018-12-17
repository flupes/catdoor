#include <Arduino.h>

#include "config.h"
#define LED_PIN 13

AdafruitIO_Feed *command = io.feed("door-cmd");
AdafruitIO_Feed *state = io.feed("door-state");

void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  int value = data->toInt();
  Serial.println(value);
  if (value == 1) {
    digitalWrite(LED_PIN, 1);
  } else {
    digitalWrite(LED_PIN, 0);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  int wait_sec = 0;
  while (!Serial) {
    delay(1000);
    wait_sec++;
    if (wait_sec > 12) break;
  }

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  Serial.println(io.connectionType());

  Serial.println(io.networkStatus());

  command->onMessage(handleMessage);

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    // Serial.println(io.statusText());
    delay(1000);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  command->get();
}

void loop() {
  static long last = millis();
  static bool flip = false;
  io.run();

  long now = millis();
  if (now - last > 6000) {
    last = now;
    if (flip) {
      flip = false;
      state->save(0);
      Serial.println("Send 0");
    } else {
      flip = true;
      state->save(1);
      Serial.println("Send 1");
    }
  }
  delay(100);
}
