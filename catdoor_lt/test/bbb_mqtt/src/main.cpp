#include <Arduino.h>

// Before including PubSubClient in order to override the default values
#define MQTT_KEEPALIVE 90
#define MQTT_SOCKET_TIMEOUT 30

#include <PubSubClient.h>
#include <SPI.h>  // because of bug in Wifi101...
#include <Servo.h>
#include <WiFi101.h>

#include "m0_hw_servo.h"
#include "netcfg.h"
#include "utils.h"
#include "watchdog_timer.h"

#define MAX_JAMMED_COUNT 5
#define UPDATE_MOT_PERIOD_MS 1000 * 2
#define UPDATE_NET_PERIOD_MS 1000 * 20
#define WIFI_CONNECT_RETRY 10
#define RECONNECT_PERIOD_SEC 20
#define CLIENT_NAME "DeckDoor"

// Topic names definition
#define TOPIC_MESSAGE "/deckdoor/message"
#define TOPIC_STATE "/deckdoor/state"
#define TOPIC_MASTER "/doormaster/command"

const uint32_t kClosedVoltageCount = 800;
const uint32_t kOpenVoltageCount = 2900;

M0HwServo servo(M0HwServo::D5, A1);

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

long g_lastReconnectAttempt = 0;
long g_lastMasterUpdate = 0;

enum LockState { CLOSED = 0, OPEN = 1, JAMMED = 2 };
static const char* STATE_NAMES[] = {"CLOSED", "OPEN", "JAMMED"};

LockState g_desiredDoorState = CLOSED;

void connect_wifi() {
  uint8_t nbtry = 0;
  int status = WL_IDLE_STATUS;  // the WiFi radio's status
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    wdt_reset();
    // We do not get out of this loop until connected.
    // However, if the Wifi.begin block for more than SETUP_WATCHDOG
    // then the board resets.
    PRINT("Attempting to connect to WEP network, SSID: ");
    PRINTLN(MY_WIFI_SSID);
    status = WiFi.begin(MY_WIFI_SSID, MY_WIFI_PASS);
    if (status == WL_CONNECTED) {
      PRINT("IP address: ");
      IPAddress ip = WiFi.localIP();
      PRINTLN(ip);
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
  }
  PRINT("in connect, status = ");
  PRINT(status);
  PRINT(" | verification: ");
  PRINTLN(WiFi.status());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_MASTER) == 0) {
    PRINT("Received an order from master : [");
    char buffer[32];
    strncpy(buffer, (char*)payload, length);
    PRINT(buffer);
    PRINTLN("]");
    // Master sends a very basic topic: a byte!
    // Anything else than 0x01 is considered a CLOSED
    g_desiredDoorState = CLOSED;
    if (((char*)payload)[0] == '1') {
      g_desiredDoorState = OPEN;
    }
    g_lastMasterUpdate = millis();
  } else {
    PRINTLN("Received message with unknown topic");
  }
}

bool connect_client() {
  int status = wifi_client.status();
  // PRINT("wifi status = ");
  // PRINTLN(status);
  if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST) {
    PRINTLN("Wifi is not connected anymore: will reboot now!");
    wdt_system_reset();
  }
  if (mqtt_client.connected()) {
    return true;
  }
  PRINT("Attempting MQTT connection... ");
  wdt_reset();
  if (mqtt_client.connect(CLIENT_NAME)) {
    PRINTLN("connected.");
    mqtt_client.subscribe(TOPIC_MASTER);
    mqtt_client.setCallback(callback);
  } else {
    PRINT(" failed (rc=");
    PRINT(mqtt_client.state());
  }
  return mqtt_client.connected();
}

void setup() {
  servo.Configure(450, 0, 2050, 120);
  // We always start in a closed state!
  // for our setup closed is MIN position, but be careful
  // to adjust is setup changes!
  servo.SetDegreeAngle(servo.GetMinDegreeAngle());

  Serial.begin(115200);
  int wait_sec = 0;
  while (!Serial) {
    delay(1000);
    wait_sec++;
    if (wait_sec > 12) break;  // wait no more than 12s for a connection...
  }

  wdt_configure(11);

  // Setup Wifi System
  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  PRINTLN("Connecting to the Wifi module...");
  if (WiFi.status() == WL_NO_SHIELD) {
    PRINTLN("WiFi shield not present");
    return;  // don't continue
  }
  connect_wifi();

  // Setup MQTT Client
  wdt_reset();
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  connect_client();
  const char* msg = "Catdoor LT started";
  mqtt_client.publish(TOPIC_MESSAGE, msg);

  g_lastMasterUpdate = millis();
}

void loop() {
  static const int closed_deg = servo.GetMinDegreeAngle();
  static const int open_deg = servo.GetMaxDegreeAngle();

  static long elapsed_mot = millis();
  static long elapsed_net = millis();
  static int jammed_counter = MAX_JAMMED_COUNT;
  static LockState lock_state = JAMMED;

  long now = millis();

  if (now - g_lastMasterUpdate > 45000) {
    // close if no update from master
    g_desiredDoorState = CLOSED;
  }

  if (now - elapsed_mot > UPDATE_MOT_PERIOD_MS) {
    elapsed_mot = now;
    uint32_t current_voltage_count = servo.GetAveragedAnalogFeedback(3);

    jammed_counter--;
    if (current_voltage_count < kClosedVoltageCount) {
      lock_state = CLOSED;
      jammed_counter = MAX_JAMMED_COUNT;
    }
    if (current_voltage_count > kOpenVoltageCount) {
      lock_state = OPEN;
      jammed_counter = MAX_JAMMED_COUNT;
    }
    if (jammed_counter < 1) {
      lock_state = JAMMED;
      PRINTLN("JAMMED!!!");
    }

    if (g_desiredDoorState != lock_state) {
      if (g_desiredDoorState == OPEN) {
        servo.SetDegreeAngle(open_deg);
      } else {
        servo.SetDegreeAngle(closed_deg);
      }
    }
  }

  if (now - elapsed_net > UPDATE_NET_PERIOD_MS) {
    elapsed_net = now;
    PRINT("door state = ");
    PRINTLN(STATE_NAMES[lock_state]);
    mqtt_client.publish(TOPIC_STATE, STATE_NAMES[lock_state]);

    float measuredvbat = analogRead(A7);
    measuredvbat *= 2;     // we divided by 2, so multiply back
    measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
    measuredvbat /= 4096;  // convert to voltage (12 bits)
    PRINTLN("VBat: ");
    PRINTLN(measuredvbat);
  }

  if (!mqtt_client.connected()) {
    PRINTLN("client connection lost");
    if (now - g_lastReconnectAttempt > 1000 * RECONNECT_PERIOD_SEC) {
      if (connect_client()) {
        PRINTLN("client reconnected");
        g_lastReconnectAttempt = 0;
      }
    }
  }

  mqtt_client.loop();
  delay(200);
  wdt_reset();
}
