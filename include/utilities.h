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

#endif // UTILITIES_H__