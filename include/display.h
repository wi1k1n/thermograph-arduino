#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <memory>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DISPLAY_WHITE WHITE
#define DISPLAY_BLACK BLACK

class Display {
    std::unique_ptr<Adafruit_SSD1306> _display;
public:
    bool init(uint16_t width, uint16_t height, TwoWire* wire, uint8_t rst, uint8_t addr);

    void display();
    void clear();
    void drawPixel(int16_t x, int16_t y, uint16_t clr = DISPLAY_WHITE);
    void displayPixel(int16_t x, int16_t y, uint16_t clr = DISPLAY_WHITE);
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t clr = DISPLAY_WHITE, uint16_t bg = DISPLAY_BLACK);
    void displayBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t clr = DISPLAY_WHITE, uint16_t bg = DISPLAY_BLACK);
};

class DisplayLayout {
protected:
    Display* _display;
public:
    DisplayLayout() = default;
    bool init(Display* display);
    virtual void draw() { }
};

class DLayoutWelcome : public DisplayLayout {
public:
    void draw() override;
};

class DLayoutMain : public DisplayLayout {
public:
    void draw() override;
};

#endif // DISPLAY_H__