#ifndef MAIN_H__
#define MAIN_H__

#include "Arduino.h"
#include "sdk.h"
#include "display.h"
#include "sensor.h"
#include "interact.h"
#include "utilities.h"
#include "TimerLED.h"
#include <vector>
#include <memory>

class Application {
	Display _display;
	DLTransition _dltransMain;
	TimerLED _displayErrorTimerLED;

	std::vector<std::unique_ptr<DisplayLayout>> _dLayouts;
	DisplayLayoutKeys _dLayoutActiveKey = DisplayLayoutKeys::WELCOME;

	PushButton _btn1;
	PushButton _btn2;
	TempSensor _sensorTemp;

	void measureTemperature();
	void showDisplayError();
public:
	bool setup();
	void loop();

	inline DisplayLayout* getActiveDisplayLayout() { return _dLayouts[_dLayoutActiveKey].get(); }
	void activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style = DLTransitionStyle::AUTO);
};

#endif // MAIN_H__