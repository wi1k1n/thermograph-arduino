#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "sdk.h"
#include "sensor.h"

#include <memory>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DISPLAY_WHITE WHITE
#define DISPLAY_BLACK BLACK

class Display {
public:
    enum ScrollType {
        NONE = 0,
        WRAP,
        FILL_BLACK
    };
    enum ScrollDir {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN
    };
    
    Display() = default;
    bool init(TwoWire* wire, uint8_t rst, uint8_t addr);
    void disable();

    Adafruit_SSD1306* operator->();
    void scroll(uint8_t amount = 1, ScrollDir dir = ScrollDir::LEFT, ScrollType scrollType = ScrollType::FILL_BLACK);

    inline bool isAvailable() const { return _available; }
    inline uint8_t rawWidth() const { return 128; }
    inline uint8_t rawHeight() const { return 64; }
private:
    std::unique_ptr<Adafruit_SSD1306> _display;
    bool _available = false;

    void scrollHorizontally(uint8_t amount, bool left, ScrollType scrollType);
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

class DLTransition {
    Display& _display;
    DisplayLayout& _displayLayout1;
    DisplayLayout& _displayLayout2;
    uint8_t* buffer = nullptr;
public:
    DLTransition(Display& display, DisplayLayout& layoutFrom, DisplayLayout& layoutTo)
    : _display(display), _displayLayout1(layoutFrom), _displayLayout2(layoutTo) {
        *(_display->getBuffer()+1) = 0xFF;
    }
};

#endif // DISPLAY_H__