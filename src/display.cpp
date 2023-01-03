#include "sdk.h"
#include "display.h"
#include "logo.h"

bool Display::init(uint16_t width, uint16_t height, TwoWire* wire, uint8_t rst, uint8_t addr) {
    _display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(width, height, wire, rst));
    if (_display == nullptr) {
        return false;
    }
    bool succeeded = _display->begin(SSD1306_SWITCHCAPVCC, addr);
    if (!succeeded) {
        return false;
    }
    _display->cp437(true);
    return true;
}

Adafruit_SSD1306* Display::operator->() {
    return _display.get();
}

/////////////////////

bool DisplayLayout::init(Display* display) {
    if (!display)
        return false;
    _display = display;
    return true;
}

void DLayoutWelcome::draw() {
    display()->drawBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, DISPLAY_WHITE);
    display()->display();
}

void DLayoutMain::update(void* data) {
    TempSensorData* tempData = static_cast<TempSensorData*>(data);
    _temp1 = tempData->temp;
}
void DLayoutMain::draw() {
    char buffer[16];
    dtostrf(_temp1, 6, 2, buffer);

    display()->clearDisplay();
    display()->setTextColor(DISPLAY_WHITE);
    
    display()->setCursor(0, DISPLAY_LAYOUT_PADDING_TOP);
    display()->setTextSize(3);
    display()->print(buffer);

    display()->setCursor(display()->getCursorX(), display()->getCursorY() - 4);
    display()->setTextSize(2);
    display()->print("o");

    display()->display();
}