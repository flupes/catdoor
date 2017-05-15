# catdoor
Cat door opening detection and notification

## First version

Notification of cat entry only (catdoor locked so that the cat can never go out, but always come in). This allows to make sure the cat in inside before dark in case we are coming late home.

### Sensing

Based on a proximity sensor looking down where the door opens. Some reflective on the side of the door allow a ver good signature, sorting out the cat. In fact, test of reflectivity on the cat hairs shows very poor reflection. Tuning the system as been done by observing serveral opening (look in the data directory)

- Microcontroller with ESP8266 Wifi:[Adafruit Feather HUZZAH with ESP8266 WiFi](https://www.adafruit.com/product/2821)
- Breadkout sensor board: [VCNL4010 Proximity/Light sensor](https://www.adafruit.com/product/466)

### Notification

The Feather HUZZAH sends a mqtt message to a Beagle Bone Black (BBB), which in turn sends notifications to a cell phone using Push Bullet.

The HUZZAH sends periodic updates of the door status, thus the BBB is can also detect potential anomalies.

This architecture with the HUZZAH + BBB never exposes the ESP8266 on the internet (only local communication).

## Second version

### Goals

- Outgoing locking mechanism that only allows the cat to go out during full daylight
- Notification of door actuation in the two direction (in and out)

Design ideas in ./design.md
