#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <memory>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "sensor.h"

#define DISPLAY_WHITE WHITE
#define DISPLAY_BLACK BLACK

class Display {
    std::unique_ptr<Adafruit_SSD1306> _display;
public:
    bool init(uint16_t width, uint16_t height, TwoWire* wire, uint8_t rst, uint8_t addr);

    Adafruit_SSD1306* operator->();
};

class DisplayLayout {
protected:
    Display* _display;
    inline Display& display() { return *_display; }
public:
    DisplayLayout() = default;
    bool init(Display* display);
    virtual void draw() { }
    virtual void update(void* data) { }
};

class DLayoutWelcome : public DisplayLayout {
public:
    void draw() override;
};

class DLayoutMain : public DisplayLayout {
    float _temp1;
public:
    void draw() override;
    void update(void* data) override;
};


class DisplayMenu {

};

#endif // DISPLAY_H__