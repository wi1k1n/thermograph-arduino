#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "sdk.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <memory>

class DisplayInterface;

class DisplayView {
protected:
    std::shared_ptr<DisplayInterface> _display{ nullptr };

public:
    bool setup(DisplayInterface* display);

    virtual bool display();
};

class DisplayViewStatus : public DisplayView {
public:
    bool display() override;
};

class DisplayInterface : public SetupBase {
    DisplayViewStatus _dvStatus;

protected:
    uint16_t _width{ DISPLAY_SCREEN_WIDTH };
    uint16_t _height{ DISPLAY_SCREEN_HEIGHT };

public:
    virtual bool display() { return true; };

    uint16_t getWidth() const { return _width; }
    uint16_t getHeight() const { return _height; }
};

class DisplaySSD1306 : public DisplayInterface {
    std::unique_ptr<Adafruit_SSD1306> _display{ nullptr };

public:
    bool setup() override;
    
    bool display() override;
};

#endif // DISPLAY_H__