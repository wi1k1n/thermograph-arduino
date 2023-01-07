#ifndef UTILITIES_H__
#define UTILITIES_H__

#include "TimerMs.h"
#include "VirtualButton.h"

class LED {
	uint8_t _pin = 0;
	bool _val = false;
public:
	LED() = default;
	bool init(uint8_t pin, bool val = false) { _pin = pin; pinMode(pin, OUTPUT); write(val); return true; }
	void on() { write(true); }
	void off() { write(false); }
	void toggle() { write(!_val); }
	void write(bool val) { _val = val; digitalWrite(_pin, val ? LOW : HIGH); }
};

#define PUSHBUTTON_PIN_UNASSIGNED 0xff
class PushButton : public VButton {
	uint8_t _pin = PUSHBUTTON_PIN_UNASSIGNED;
public:
	PushButton() = default;
	PushButton(uint8_t pin) {
		init(pin);
	}
	bool init(uint8_t pin) {
		_pin = pin;
		pinMode(_pin, INPUT_PULLUP);
		return true;
	}
	bool tick() {
		return poll(!digitalRead(_pin));
	}
};

class Timer : public TimerMs {
public:
	enum MODE {
		PERIOD = 0, // repeats
		TIMER = 1	// triggers once
	};

	Timer() = default;
	bool init(uint32_t prd = 1000, MODE mode = MODE::PERIOD) {
		setTime(prd);
		if (mode == MODE::TIMER) {
			setTimerMode();
		} else {
			setPeriodMode();
		}
		return true;
	}
};

template <typename T>
struct Point2 {
	T x;
	T y;
	Point2() : x(T()), y(T()) { }
	Point2(T v) : x(v), y(v) { }
	Point2(T x, T y) : x(x), y(y) { }
};
typedef Point2<int16_t> Point;
typedef Point2<uint16_t> UPoint;
typedef Point2<int8_t> Point8_t;
typedef Point2<uint8_t> UPoint8_t;
typedef Point2<float> PointF;

#endif // UTILITIES_H__