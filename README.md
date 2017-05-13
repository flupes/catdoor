# catdoor
Cat door opening detection and notification

## First version

Notification of entry only (catdoor locked so that the cat can never go out, but always come in).

Based on a proximity sensor looking down where the door opens. Some reflective on the side of the door allow a ver good signature, sorting out the cat. In fact, test of reflectivity on the cat hairs shows very poor reflection. Tuning the system as been done by observing serveral opening (look in the data directory)

- Microcontroller with ESP8266 Wifi:[Adafruit Feather HUZZAH with ESP8266 WiFi](https://www.adafruit.com/product/2821)
- Breadkout sensor board: [VCNL4010 Proximity/Light sensor](https://www.adafruit.com/product/466)
