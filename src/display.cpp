#include "sdk.h"
#include "display.h"
#include "display_layout.h"

Display::~Display() {
	disable();
}

bool Display::init(TwoWire* wire, uint8_t rst, uint8_t addr) {
	_display = std::unique_ptr<Adafruit_SSD1306>(new Adafruit_SSD1306(rawWidth(), rawHeight(), wire, rst));
	if (_display == nullptr || !_display->begin(SSD1306_SWITCHCAPVCC, addr)) {
		_timerLEDError.init(LED_BUILTIN);
		_timerLEDError.setIntervals(4, (const uint16_t[]){ 50, 100, 150, 100 });
		_timerLEDError.restart();
		return false;
	}
	_available = true;
	_display->cp437(true);

	return true;
}
void Display::disable() {
	_display.reset();
	_available = false;
}
void Display::tick() {
	if (_timerLEDError.isActive())
		_timerLEDError.tick();
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
			// TODO: implement! (can be not easy due to memory layout)
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
	
	// TODO: memory manipulation should be encapsulated by Display class!!!

	// temporarily save current buffer (because it may differ from what we get after draw(), but we also need to prerender layout2)
	uint8_t* tempBuff = new uint8_t[bufferSize];
	memcpy(tempBuff, display()->getBuffer(), bufferSize);

	// draw second layout first and move to _buffer
	_displayLayout2->draw(false);
	memcpy(_buffer, display()->getBuffer(), bufferSize);

	memcpy(display()->getBuffer(), tempBuff, bufferSize);
	delete[] tempBuff;

	_displayLayout1->deactivate();
	
	_startedTimestamp = millis();
	tick();
}
void DLTransition::stop() {
	_isRunning = false;
	delete[] _buffer;
}

float DLTransition::interpolate(float a, float b, float x, Interpolation style) {
	float t = 0;
	switch (style) {
		case Interpolation::APOW3:
			t = a + (b - a) * powf(x, 1.f / 3.f);
			break;
		default:
			t = a + x * (b - a);
			break;
	}
	return constrain(t, a, b);
}
void DLTransition::tick() {
	if (!isRunning()) {
		return;
	}
	unsigned long curTime = millis();
	float t = interpolate(0.f, 1.f, (float)(curTime - _startedTimestamp) / _duration, _interpolation);
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
		_displayLayout2->activate();
		return;
	}
}