#include "sdk.h"
#include "display.h"
#include "logo.h"
#include "main.h"

Display::~Display() {
    disable();
}

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

// src is a memory block with the same layout as _display->getBuffer()
void Display::scrollHorizontally(uint8_t amount, bool left, ScrollType scrollType, const uint8_t* src, uint8_t srcOffset) {
    if (amount == 0) {
        return;
    }

    uint8_t shiftingWidth = rawWidth() - amount;
    uint8_t* bufferDisplay = _display->getBuffer();
    uint8_t* bufferWrap = nullptr;
    bool wrap = scrollType == ScrollType::WRAP;
    if (wrap) {
        bufferWrap = new uint8_t[amount];
    }
    bool copy = wrap || scrollType == ScrollType::COPY; // if empty space should be filled from some buffer, either bufferWrap or src
    bool fillBlack = scrollType == ScrollType::FILL_BLACK;

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

    for (uint8_t rowIdx = 0; rowIdx < rawHeight() / pixelDepth(); ++rowIdx) {
        uint8_t* row = bufferDisplay + rowIdx * rawWidth();
        const uint8_t* rowSrc = src + rowIdx * rawWidth();

        uint8_t* ptrMoveFrom = row + moveFromDir; // pointer from where data that is moved begins
        uint8_t* ptrMoveTo = row + moveToDir; // pointer to where data that is moved begins
        uint8_t* ptrWrapFrom = row + wrapFromDir; // pointer from where wrapped data begins
        uint8_t* ptrWrapTo = row + wrapToDir; // pointer to where wrapped data is copied or where the fill_black block starts
        if (copy) {
            const uint8_t* copySrc = wrap ? bufferWrap : (rowSrc + srcOffset); // pointer to where copy data from
            if (wrap) {
                memcpy(bufferWrap, ptrWrapFrom, amount);
            }
            memmove(ptrMoveTo, ptrMoveFrom, shiftingWidth);
            memcpy(ptrWrapTo, copySrc, amount);
        } else {
            memmove(ptrMoveTo, ptrMoveFrom, shiftingWidth);
            if (fillBlack) {
                memset(ptrWrapTo, 0x00, amount);
            }
        }
    }
    delete[] bufferWrap;
}

void Display::scroll(uint8_t amount, ScrollDir dir, ScrollType scrollType, const uint8_t* src, uint8_t srcOffset) {
    // SSD1306 GDDRAM consists of 8 pages of size (8x128) spatially placed one under another (and consequently in memory)
    // Does not care about rotation!
    switch (dir) {
        case ScrollDir::LEFT:
            scrollHorizontally(amount, true, scrollType, src, srcOffset);
            break;
        case ScrollDir::RIGHT:
            scrollHorizontally(amount, false, scrollType, src, srcOffset);
            break;
        default:
            LOGLN(F("Vertical scroll is not implemented yet!"));
            break;
    }
}

/////////////////////

DLTransition::~DLTransition() {
    delete[] _buffer;
}

bool DLTransition::init(Display* display, uint16_t duration, Interpolation interpolation) {
    if (!display) {
        DLOGLN(F("_display is nullptr! Abroted."));
        return false;
    }
    _display = display;
    _duration = duration;
    _interpolation = interpolation;
    return true;
}

void DLTransition::start(DisplayLayout* layoutFrom, DisplayLayout* layoutTo, Display::ScrollDir direction) {
    if (!layoutFrom || !layoutTo) {
        DLOGLN(F("layoutFrom or layoutTo is nullptr! Abroted."));
        return;
    }
    if (!_display) {
        DLOGLN(F("_display is nullptr! Abroted."));
        return;
    }
    if (isRunning()) {
        stop();
    }
    
    _displayLayout1 = layoutFrom;
    _displayLayout2 = layoutTo;
    _direction = direction;

    _isRunning = true;
    _curBuffersShift = 0;
    uint16_t bufferSize = _display->rawWidth() * _display->rawHeight() / _display->pixelDepth();
    _buffer = new uint8_t[bufferSize];
    
    // draw second layout first and move to _buffer
    _displayLayout2->draw(false);
    memmove(_buffer, display()->getBuffer(), bufferSize);
    // draw second layout and keep it in display
    _displayLayout1->draw(false);
    
    _startedTimestamp = millis();
    tick();
}
void DLTransition::stop() {
    _isRunning = false;
    delete[] _buffer;
}
void DLTransition::tick() {
    if (!isRunning()) {
        return;
    }
    unsigned long curTime = millis();
    float t = (float)(curTime - _startedTimestamp) / _duration;
    // amount - how far current buffer should be shifted at current millis() time
    uint8_t amount = min(static_cast<uint8_t>(t * _display->rawWidth()), _display->rawWidth());
    // how much more buffers should be shifted considering their current shift
    uint8_t amountForCurBuffers = amount - _curBuffersShift;
    if (!amountForCurBuffers) {
        return;
    }

    uint8_t bufferOffset = _direction == Display::ScrollDir::LEFT ? _curBuffersShift : _display->rawWidth() - amount;
    _display->scroll(amountForCurBuffers, _direction, Display::ScrollType::COPY, _buffer, bufferOffset);
    display()->display();
    
    _curBuffersShift += amountForCurBuffers;
    if (curTime - _startedTimestamp >= _duration) {
        stop();
        _displayLayout2->draw();
        return;
    }
}


bool DisplayLayout::init(Display* display, Application* app) {
    if (!display || !app)
        return false;
    _display = display;
    _app = app;
    return true;
}

void DLayoutWelcome::draw(bool doDisplay) {
    DLOGLN();
    display()->clearDisplay();

    display()->drawBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, DISPLAY_WHITE);

    if (doDisplay) {
        display()->display();
    }
}

void DLayoutMain::update(void* data) {
    TempSensorData* tempData = static_cast<TempSensorData*>(data);
    _temp1 = tempData->temp;
}
void DLayoutMain::input(PushButton& btn1, PushButton& btn2) {
    if (btn1.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
        return;
    }
    if (btn2.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
        return;
    }
}
void DLayoutMain::draw(bool doDisplay) {
    DLOGLN();
    display()->clearDisplay();

    display()->setTextColor(DISPLAY_WHITE);

    char buffer[16];
    dtostrf(_temp1, 6, 2, buffer);
    
    display()->setCursor(0, DISPLAY_LAYOUT_PADDING_TOP);
    display()->setTextSize(3);
    display()->print(buffer);

    display()->setCursor(display()->getCursorX(), display()->getCursorY() - 4);
    display()->setTextSize(2);
    display()->print(F("o"));
    
    display()->setCursor(0, 0);
    display()->setTextSize(1);
    display()->print(F("Thermograph v2"));

    if (doDisplay) {
        display()->display();
    }
}


void DLayoutGraph::update(void* data) {

}
void DLayoutGraph::input(PushButton& btn1, PushButton& btn2) {
    if (btn1.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
        return;
    }
    if (btn2.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
        return;
    }
}
void DLayoutGraph::draw(bool doDisplay) {
    DLOGLN();
    display()->clearDisplay();
    
    display()->setCursor(0, 0);
    display()->setTextSize(1);
    display()->print(F("Graph Layout"));

    if (doDisplay) {
        display()->display();
    }
}

void DLayoutSettings::update(void* data) {

}
void DLayoutSettings::input(PushButton& btn1, PushButton& btn2) {
    if (btn1.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
        return;
    }
    if (btn2.click()) {
        _app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
        return;
    }
}
void DLayoutSettings::draw(bool doDisplay) {
    DLOGLN();
    display()->clearDisplay();
    
    display()->setCursor(0, 0);
    display()->setTextSize(1);
    display()->print(F("Settings Layout"));

    if (doDisplay) {
        display()->display();
    }
}