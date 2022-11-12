#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "sdk.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <memory>

class DisplayView {

};

template <class DisplayImpl>
class Display : public SetupBase {
    DisplayImpl _display;

    DisplayView _dvWelcome;
    DisplayView _dvIdle;
public:
    bool setup() override;
};

class DisplayInterface : public SetupBase {
protected:
    uint16_t _width{ DISPLAY_SCREEN_WIDTH };
    uint16_t _height{ DISPLAY_SCREEN_HEIGHT };
public:
};

class DisplaySSD1306 : public DisplayInterface {
    std::unique_ptr<Adafruit_SSD1306> _display{ nullptr };
public:
    bool setup() override {
        DisplayInterface::setup();

        _display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(_width, _height, &Wire, DISPLAY_PIN_RESET));
        if (!_display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            DLOGLN(F("Couldn't start up the display!"));
            return false;
        }
        return true;
    }
};

template class Display<DisplaySSD1306>;

#endif // DISPLAY_H__