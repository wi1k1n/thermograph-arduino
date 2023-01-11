#ifndef MAIN_H__
#define MAIN_H__

#include "Arduino.h"
#include "sdk.h"
#include "display.h"
#include "display_layout.h"
#include "sensor.h"
#include "interact.h"
#include "utilities.h"
#include <vector>
#include <memory>

class Application {
	enum Mode {
		BACKGROUND = 0,				// autonomously woke up while working in background
		BACKGROUND_INTERRUPTED,		// background job in progress but user interrupt
		INTERACT,					// no background job in process, fully interact
#ifdef TDEBUG
		_DEBUG_LITTLEFS_EXPLORER,
#endif
	};

	Mode _mode = Mode::BACKGROUND;

	Display _display;
	DLTransition _dltransMain;

	std::vector<std::unique_ptr<DisplayLayout>> _dLayouts;
	DisplayLayoutKeys _dLayoutActiveKey = DisplayLayoutKeys::NONE;

	PushButton _btn1;
	PushButton _btn2;
	TempSensor _sensorTemp;

	void showDisplayError();
public:
	bool setup();
	void loop();

	void makeMeasurement();
	
	inline DisplayLayout* getActiveDisplayLayout() { return _dLayouts[_dLayoutActiveKey].get(); }
	void activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style = DLTransitionStyle::AUTO);
	
	inline bool isModeBackground() const { return _mode == Mode::BACKGROUND; }
	inline bool isModeBackgroundInterrupted() const { return _mode == Mode::BACKGROUND_INTERRUPTED; }
	inline bool isModeInteract() const { return _mode == Mode::INTERACT; }
	inline void setModeBackgroundInterrupted() { _mode = Mode::BACKGROUND_INTERRUPTED; }
	inline void setModeInteract() { _mode = Mode::INTERACT; }

	inline bool isInteractionAvailable() const { return isModeBackgroundInterrupted() || isModeInteract() ;}
};

#endif // MAIN_H__