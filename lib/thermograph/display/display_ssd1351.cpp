#include "display_ssd1351.h"
#include "sdk.h"
#include "configuration.h"

bool DisplaySSD1351::init()
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

void DisplaySSD1351::tick() {
}

void DisplaySSD1351::disable() {
}

bool DisplaySSD1351::isEnabled() const {
	return true;
}
