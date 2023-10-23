#ifndef APPLICATION_H__
#define APPLICATION_H__

#include "Arduino.h"
#include "sdk.h"
#include "settings.h"
#include "display/display.h"
#include "display/display_layout.h"
#include "sensor.h"
#include "utilities.h"
#include <vector>
#include <memory>

class Application {
public:
	bool setup();
	void loop();

	void makeMeasurement(bool storeData = false);
	bool sleep();
	bool startBackgroundJob();
	bool stopBackgroundJob();

	inline DisplayLayout* getActiveDisplayLayout() {
		if (_dLayoutActiveKey == DisplayLayoutKeys::NONE || _dltransMain.isRunning())
			return nullptr;
		return _dLayouts[_dLayoutActiveKey].get();
	}
	DisplayLayoutKeys getActiveLayoutKey() const { return _dltransMain.isRunning() ? DisplayLayoutKeys::NONE : _dLayoutActiveKey; }
	bool isActiveLayoutKey(DisplayLayoutKeys key) { return getActiveLayoutKey() == key; }
	void activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style = DLTransitionStyle::AUTO, bool force = false);

	// This timer is only for live preview of the temperature while in interactive mode
	void setRealtimeMeasurementPeriod(uint32_t period) { _realtimeMeasurementTimer.setTime(period); }
	void stopRealtimeMeasurement() { _realtimeMeasurementTimer.stop(); }
	void startRealtimeMeasurement(bool triggerInstanly = false) {
		_realtimeMeasurementTimer.start();
		if (triggerInstanly)
			_realtimeMeasurementTimer.force();
	}
	
	inline bool isModeBackground() const { return _mode == Mode::BACKGROUND; }
	inline bool isModeInteract() const { return _mode == Mode::INTERACT; }
	inline void setModeInteract() {
		DLOGLN("SetMode(INTERACT)");
		_mode = Mode::INTERACT;
	}
	inline void setModeBackground() {
		DLOGLN("SetMode(BACKGROUND)");
		_mode = Mode::BACKGROUND;
	}

	inline bool isInProgress() const { return _isInProgress; }
	inline void setInProgress(bool val) {
		DLOG("SetInProgress(");
		LOG(val ? "true" : "false");
		LOGLN(")");
		_isInProgress = val;
	}

	inline const ThSettings& getSettings() const { return _settings; }
	inline ThSettings& getSettings() { return _settings; }

	enum Mode {
		BACKGROUND = 0,				// autonomously woke up while working in background
		INTERACT,					// no background job in process, fully interact
#ifdef TDEBUG
		_DEBUG_LITTLEFS_EXPLORER,
#endif
	};

private:
	bool initDisplayStuff();

	void showDisplayError();
	bool debugInitLFSExplorer();

private:
	Mode _mode = Mode::BACKGROUND;

	Display _display; 													// Wrapped hardware display
	DLTransition _dltransMain; 											// Transition class for the display

	std::vector<std::unique_ptr<DisplayLayout>> _dLayouts; 				// Array of display layouts to be used with _display
	DisplayLayoutKeys _dLayoutActiveKey = DisplayLayoutKeys::NONE; 		// Active display layout that is being showed on _display atm

	HardwareInputs _inputs; 											// Hardware inputs manager (e.g. pushbuttons)
	ThSettings _settings; 												// Device settings manager (e.g. timer intervals, temperature units etc.)
	
	TempSensor _sensorTemp; 											// Temperature sensor

	TimerMs _realtimeMeasurementTimer; 									// Timer for the real-time measurements (those that are shown 'live' on main display layout)
	bool _isInProgress = false; 										// Flag showing if current task of storing data is in progress

	size_t _timeSinceStarted = 0; 										// Timestemp from isSleeping structure, which designates the amount of time spent from when the task started
};

#endif // APPLICATION_H__