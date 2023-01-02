#include "sdk.h"
#include "display.h"
#include "logo.h"

bool Display::init(uint16_t width, uint16_t height, TwoWire* wire, uint8_t rst, uint8_t addr) {
    _display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(width, height, wire, rst));
    if (_display == nullptr) {
        return false;
    }
    return _display->begin(SSD1306_SWITCHCAPVCC, addr);
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t clr) {
    _display->drawPixel(x, y, clr);
}
void Display::displayPixel(int16_t x, int16_t y, uint16_t clr) {
    drawPixel(x, y, clr);
    display();
}

void Display::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t clr, uint16_t bg) {
    _display->drawBitmap(x, y, bitmap, w, h, clr, bg);
}
void Display::displayBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t clr, uint16_t bg) {
    drawBitmap(x, y, bitmap, w, h, clr, bg);
    display();
}


void Display::clear() {
    _display->clearDisplay();
}
void Display::display() {
    _display->display();
}

/////////////////////

bool DisplayLayout::init(Display* display) {
    if (!display)
        return false;
    _display = display;
    return true;
}

void DLayoutMain::draw() {

}

void DLayoutWelcome::draw() {
    _display->displayBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, DISPLAY_WHITE);
}