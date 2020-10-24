## Thermograph

Build a arduino-based thermometer.

## Hardware

- Arduino Nano (or Uno)
- 2 buttons
- Themistor
- A few resistors

## Wiring

(Coming soon)

## TODO list

- USB mode
- Optimize memory usage
    - Display lib takes ~200 bytes of SRAM (+1KB is allocated dynamically on Heap)
    - Timers don't need all the internal variables (now each takes 13 bytes => 65 bytes in total)
    - Each GButton take ~23 bytes (could be less?), so ~46 bytes in total
- Optimize flash usage
    - remove adafruit splash logo

## Known bugs

- After [Load] the range is wrong
- On [Save] theres high eeprom wear (need to consequently slide cursor)
- Time estimations r not accurate (should be rounded instead of flooring)
- No time memory (only considers current graph timeout setting, even if the whole graph has been built with different graph timeout)
- Low memory
## 