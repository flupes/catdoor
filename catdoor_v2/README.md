# Intro

This is the source directory for the version 2 of the catdoor application.

The code under this directory is structured to be compiled with platformio.

```
catdoor_v2/
├── app            main catdoor application
├── libs           libraries
└── tests          test programs (development purpose)
```                  

# Dependencies

## libs via platformio
Install the following:
- WiFi101 by Arduini - http://www.arduino.cc/en/Reference/WiFi101
- PubSubClient by Nick O'Leary - https://github.com/knolleary/pubsubclient.git

## extra libs

To get extra custom libraries like my fork of RTClib, use:
```
git submodules init
git submodules update
```

# Build and Install

Compile with:

    pio run -v

Upload with:

    pio run -v --target upload

Serial Monitor:

    pio device monitor --port COM29 --baud 115200
