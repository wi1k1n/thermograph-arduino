#include "sdk.h"
#include "display.h"
#include "logo.h"

bool Display::init(TwoWire* wire, uint8_t rst, uint8_t addr) {
    _display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(rawWidth(), rawHeight(), wire, rst));
    if (_display == nullptr) {
        return false;
    }
    _available = _display->begin(SSD1306_SWITCHCAPVCC, addr);
    if (!_available) {
        return false;
    }

    _display->cp437(true);

    return true;
}
void Display::disable() {
    _display.reset();
    _available = false;
}

Adafruit_SSD1306* Display::operator->() {
    return _display.get();
}

void Display::scrollHorizontally(uint8_t amount, bool left, ScrollType scrollType) {
    uint8_t shiftingWidth = 128 - amount;
    uint8_t* bufferDisplay = _display->getBuffer();
    uint8_t* bufferWrap = nullptr;
    bool wrap = scrollType == ScrollType::WRAP;
    bool fillBlack = scrollType == ScrollType::FILL_BLACK;
    if (wrap) {
        bufferWrap = new uint8_t[amount];
    }
    uint8_t moveFromDir = 0;
    uint8_t moveToDir = amount;
    uint8_t wrapFromDir = shiftingWidth;
    uint8_t wrapToDir = 0;
    if (left) {
        moveFromDir = amount;
        moveToDir = 0;
        wrapFromDir = 0;
        wrapToDir = shiftingWidth;
    }
    for (uint8_t rowIdx = 0; rowIdx < 8; ++rowIdx) {
        uint8_t* row = bufferDisplay + rowIdx * rawWidth();
        uint8_t* ptrMoveFrom = row + moveFromDir; // pointer from where data that is moved begins
        uint8_t* ptrMoveTo = row + moveToDir; // pointer to where data that is moved begins
        uint8_t* ptrWrapFrom = row + wrapFromDir; // pointer from where wrapped data begins
        uint8_t* ptrWrapTo = row + wrapToDir; // pointer to where wrapped data is copied or where the fill_black block starts
        if (wrap) {
            memcpy(bufferWrap, ptrWrapFrom, amount);
            memmove(ptrMoveTo, ptrMoveFrom, shiftingWidth);
            memcpy(ptrWrapTo, bufferWrap, amount);
        } else {
            memmove(ptrMoveTo, ptrMoveFrom, shiftingWidth);
            if (fillBlack) {
                memset(ptrWrapTo, 0x00, amount);
            }
        }
    }
    delete bufferWrap;
}

void Display::scroll(uint8_t amount, ScrollDir dir, ScrollType scrollType) {
    // SSD1306 GDDRAM consists of 8 pages of size (8x128) spatially placed one under another (and consequently in memory)
    // Does not care about rotation!
    switch (dir) {
        case ScrollDir::LEFT:
            scrollHorizontally(amount, true, scrollType);
            break;
        case ScrollDir::RIGHT:
            scrollHorizontally(amount, false, scrollType);
            break;
        default:
            LOGLN(F("Vertical scroll is not implemented yet!"));
            break;
    }
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
    display()->print(F("o"));
    
    display()->setCursor(40, 0);
    display()->setTextSize(1);
    display()->print(F("Thermograph v2"));

    display()->display();
}