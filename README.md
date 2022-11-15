# Thermograph

This is a second iteration of the thermograph project, that haven't completely succeeded before.

## Desired Specifications

Should...

- be an autonomous device with a small display and a couple of buttons for the interaction
- be capable of permanently storing some amount of data and allowing to visualize and explore stored data
- be capable of streaming data: Serial, Wi-Fi (if allowed by hardware)
- contain internal temperature sensor such that it can be used as is (without any extra wires)
- be capable of using extended temperature sensor (ideally thermocouple e.g. MAX31855)
- be designed to not being limited from adding extra sensor (e.g. light sensor)

### Some thoughts

- ESP8266 would be a better choice (comparing to Atmega328P), because of embedded WiFi module
- The inputs are 2 (or 3) tact buttons, hence interaction is quite limited, so the UI-design should correspond to properly
- Because of the limited storage capacity (1KB for Atmega328P and 4KB for ESP8266 emulated in flash -> or better LittleFS), the storing should be optimized for capacity (using data compression, e.g. zlib, uzlib, miniz?)
- EEPROM is susceptible for number of overwrites, storing algorithm should consider it to avoid wearing of some specific memory segments (LittleFS solves this problem out of the box)
- The codebase should address the power supply is limits (e.g. using deep sleeping, when possible). Possible power supply: a couple of AA or AAA batteries

### TODO List:
- Rethink the _SetupBase_ concept