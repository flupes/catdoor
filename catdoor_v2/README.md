This is the source directory for the version 2 of the catdoor application.

The code under this directory is structured to be compiled with platformio.

```
catdoor_v2/
├── app            main catdoor application
├── libs           libraries
└── tests          test programs (development purpose)
```                  

Compile with:

    pio run -v

Upload with:

    pio run -v --target upload

Serial Monitor:

    pio device monitor --port COM29 --baud 115200
