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

	void makeMeasurement();
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
	inline bool isModeBackgroundInterrupted() const { return _mode == Mode::BACKGROUND_INTERRUPTED; }
	inline bool isModeInteract() const { return _mode == Mode::INTERACT; }
	inline void setModeBackgroundInterrupted() { _mode = Mode::BACKGROUND_INTERRUPTED; }
	inline void setModeInteract() { _mode = Mode::INTERACT; }
	inline bool isInteractionAvailable() const { return isModeBackgroundInterrupted() || isModeInteract() ;}

	inline const ThSettings& getSettings() const { return _settings; }
	inline ThSettings& getSettings() { return _settings; }

	enum Mode {
		BACKGROUND = 0,				// autonomously woke up while working in background
		BACKGROUND_INTERRUPTED,		// background job in progress but user interrupt
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

	Display _display;
	DLTransition _dltransMain;

	std::vector<std::unique_ptr<DisplayLayout>> _dLayouts;
	DisplayLayoutKeys _dLayoutActiveKey = DisplayLayoutKeys::NONE;

	HardwareInputs _inputs;
	ThSettings _settings;
	TempSensor _sensorTemp;

	TimerMs _realtimeMeasurementTimer;

	// size_t _timeAwake = 0; // how much time
};

#endif // APPLICATION_H__