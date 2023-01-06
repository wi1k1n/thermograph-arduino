#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "sdk.h"
#include "TimerLED.h"

#include <memory>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DISPLAY_WHITE WHITE
#define DISPLAY_BLACK BLACK
#define DISPLAY_FONT_WIDTH 6
#define DISPLAY_FONT_HEIGHT 8

class Application;
class Display {
public:
	enum ScrollType {
		NONE = 0,
		WRAP,
		FILL_BLACK,
		COPY
	};
	enum ScrollDir {
		LEFT = 0,
		RIGHT,
		UP,
		DOWN
	};

	Display() = default;
	~Display();
	bool init(TwoWire* wire, uint8_t rst, uint8_t addr);
	void disable();
	void tick();

	Adafruit_SSD1306* operator->();
	void scroll(uint8_t amount = 1,
				ScrollDir dir = ScrollDir::LEFT,
				ScrollType scrollType = ScrollType::FILL_BLACK,
				const uint8_t* src = nullptr,
				uint8_t srcOffset = 0);

	inline bool isAvailable() const { return _available; }
	inline uint8_t rawWidth() const { return 128; }
	inline uint8_t rawHeight() const { return 64; }
	inline uint8_t pixelDepth() const { return 8; } // in bits
private:
	std::unique_ptr<Adafruit_SSD1306> _display;
	TimerLED _timerLEDError;
	bool _available = false;

	void scrollHorizontally(uint8_t amount, bool left, ScrollType scrollType, const uint8_t* src, uint8_t srcOffset);
};

class DisplayLayout;
class DLTransition {
	// TODO: Add minimal delay between redraws on tick() function
public:
	enum Interpolation {
		LINEAR = 0,
		APOW3
	};
	DLTransition() = default;
	~DLTransition();
	bool init(Display* display, uint16_t durationMs = 250, Interpolation interpolation = Interpolation::LINEAR);

	void start(DisplayLayout* layoutFrom, DisplayLayout* layoutTo, Display::ScrollDir direction = Display::ScrollDir::LEFT);
	void tick();
	void stop();

	inline bool isRunning() const { return _isRunning; }
private:
	Display* _display = nullptr;
	DisplayLayout* _displayLayout1 = nullptr;
	DisplayLayout* _displayLayout2 = nullptr;
	uint16_t _duration;
	Display::ScrollDir _direction;
	Interpolation _interpolation;
	
	uint8_t* _buffer = nullptr;
	uint8_t _curBuffersShift = 0; // what's the 'amount' value for the buffers that is currently displayed
								  // (i.e. the distance of the border between 2 buffers to the real display edge)
	unsigned long _startedTimestamp = 0;
	// unsigned long _lastTickTimestamp = 0;
	bool _isRunning = false;

	static float interpolate(float a, float b, float x, Interpolation style = Interpolation::LINEAR);
	
	inline Display& display() const { return *_display; }
};

#endif // DISPLAY_H__