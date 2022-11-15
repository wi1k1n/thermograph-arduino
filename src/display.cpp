#include "display.h"


bool DisplayView::setup(DisplayInterface* display) {
    _display.reset(display);
    return true;
}

bool DisplayView::display() {
    return _display != nullptr && _display->isReady();
}

bool DisplayViewStatus::display() {
    if (!DisplayView::display()) {
        return false;
    }
    _display->display();
    return true;
}

bool DisplaySSD1306::setup() {
    DisplayInterface::setup();

    _display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(_width, _height, &Wire, DISPLAY_PIN_RESET));
    if (!_display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DLOGLN(F("Couldn't start up the display!"));
        return false;
    }
    return true;
}

bool DisplaySSD1306::display() {
    _display->display();
    return true;
}