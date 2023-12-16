#include "hwdisplay_ssd1351.h"
#include "sdk.h"
#include "configuration.h"

#define CHECKDISPLAYPTR do { if (!ASSERTPTR(_displayHW.get())) return; } while(0)

bool HWDisplaySSD1351::init()
{
	_displayHW = std::unique_ptr<Adafruit_SSD1351>(new Adafruit_SSD1351(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_PIN_CS, DISPLAY_PIN_DC, DISPLAY_PIN_MOSI, DISPLAY_PIN_SCLK, DISPLAY_PIN_RST));
	if (_displayHW == nullptr) {
		DLOGLN("Couldn't initialize display!");
		return false;
	}
	_displayHW->begin();
	DLOGLN("Hello", " ", "SSD1351!");
	return true;
}

void HWDisplaySSD1351::tick() {
}

void HWDisplaySSD1351::disable() {
}

void HWDisplaySSD1351::display() {
	CHECKDISPLAYPTR;
}

bool HWDisplaySSD1351::isEnabled() const {
	return true;
}

void HWDisplaySSD1351::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
	CHECKDISPLAYPTR;
	_displayHW->drawBitmap(x, y, bitmap, w, h, color);
}

void HWDisplaySSD1351::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) {
	CHECKDISPLAYPTR;
	_displayHW->drawBitmap(x, y, bitmap, w, h, color, bg);
}

void HWDisplaySSD1351::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
	CHECKDISPLAYPTR;
	_displayHW->drawBitmap(x, y, bitmap, w, h, color);
}

void HWDisplaySSD1351::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
	CHECKDISPLAYPTR;
	_displayHW->drawBitmap(x, y, bitmap, w, h, color, bg);
}
