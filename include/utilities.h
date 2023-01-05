#ifndef UTILITIES_H__
#define UTILITIES_H__

#include "TimerMs.h"

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