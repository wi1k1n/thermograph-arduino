#ifndef DISPLAY_SSD1351_H__
#define DISPLAY_SSD1351_H__

#include <Arduino.h>
#include <Adafruit_SSD1351.h>

#include <memory>

#include "display/displayinterface.h"

class DisplaySSD1351 : public DisplayInterface {
public:
	DisplaySSD1351() = default;
	
	bool init() override;
	void tick() override;
	void disable() override;

	bool isEnabled() const override;

private:
	std::unique_ptr<Adafruit_SSD1351> _displayHW;
};

#endif // DISPLAY_SSD1351_H__