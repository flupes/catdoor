/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME    "mosfet"
#define IO_KEY         "1234"

/******************************* WIFI **************************************/

#define WIFI_SSID       "****"
#define WIFI_PASS       "****"


#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
