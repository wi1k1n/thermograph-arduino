#include "application.h"

#include "filesystem.h"

#ifdef TDEBUG
#include "lfsexplorer.h"
#endif

#include "display/dlayouts/dl_bginterrupted.h"
#include "display/dlayouts/dl_measviewer.h"
#include "display/dlayouts/dl_graph.h"
#include "display/dlayouts/dl_main.h"
#include "display/dlayouts/dl_settings.h"
#include "display/dlayouts/dl_welcome.h"

using HIChannel = HardwareInputs::HardwareInputChannel;

constexpr uint8_t INITIALIZATION_STEPS = 8;
uint8_t _currentInitializationStepIdx = 1;
static void reportInitialized(const String& str) {
	DLOGLN("(" + String(_currentInitializationStepIdx++) + "/" + String(INITIALIZATION_STEPS) + ") " + str);
}

bool Application::setup() {
	_UART_.begin(115200);
	DLOGLN(F("Welcome to Thermograph v2"));

	// Minimal initialization
	{
		if (!ThFS::init())
			return false;
		reportInitialized("ThFS initialized");

		// Buttons initialization
		{
			uint8_t btnPin = INTERACT_PUSHBUTTON_1_PIN;
			if (!_inputs.init(HIChannel::BUTTON1, &btnPin))
				return false;
			btnPin = INTERACT_PUSHBUTTON_2_PIN;
			if (!_inputs.init(HIChannel::BUTTON2, &btnPin))
				return false;
			reportInitialized("Buttons initialized");
		}

	#ifdef TDEBUG // Hold btn2 to load Serial LittleFS explorer code
		if (debugInitLFSExplorer())
			return true;
	#endif

		if (!Storage::init())
			return false;
		reportInitialized("Storage manager initialized");

		if (!_settings.init(this))
			return false;
		reportInitialized("Settings manager initialized");
		
		if (!_sensorTemp.init())
			return false;
		reportInitialized("Sensor temp initialized");
	}
	
	// 
	const SStrSleeping& sleepingEntry = Storage::getSleeping(true);
	bool isSleeping = sleepingEntry.timeAwake > 0;
	if (isSleeping) { 
		DLOG(F("isSleeping == true: timeAwake = "));
		LOG(sleepingEntry.timeAwake);

		delay(MODE_DETECTION_DELAY);
		if (_inputs.tick(HIChannel::BUTTON1) && _inputs.tick(HIChannel::BUTTON2)) {
			DLOG(F("WARNING!!! Background task interrupted! Track of time was lost!"));
			setModeInteract();
		} else {
			_timeSinceStarted = isSleeping + _settings.getEntry<uint16_t>(ThSettings::Entries::PERIOD_CAPTURE) * 1000;
			LOG(" | _timeSinceStarted = ");
			LOGLN(_timeSinceStarted);

			makeMeasurement(true);
			SStrDatafile df = Storage::getDatafile();
			LOG("Data:");
			for (auto d : df.data) {
				LOG(" ");
				LOG(d);
			}
			sleep();
		}
		LOGLN("");
	} else {
		setModeInteract();
	}
	DLOG(F("Mode: "));
	LOGLN(_mode);

	if (isModeInteract())
		if (!initDisplayStuff())
			return false;
	
	if (isModeInteract()) {
		activateDisplayLayout(DisplayLayoutKeys::WELCOME, DLTransitionStyle::NONE);
	}
	// else if (isModeBackgroundInterrupted()) {
	// 	activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE);
	// 	// activateDisplayLayout(DisplayLayoutKeys::BACKGROUND_INTERRUPTED, DLTransitionStyle::NONE); // TODO: remove this display layout at all!
	// }

	return true;
}

/////////////////////////////////////////////////////////////////////////

void Application::loop() {
#ifdef TDEBUG
	if (_mode == Mode::_DEBUG_LITTLEFS_EXPLORER)
		return DEBUG::LittleFSExplorer("");
#endif

	// Measurement-related ticks
	if (_realtimeMeasurementTimer.tick()) {
		makeMeasurement();
	}

	// Storing data timer


	// Display-related ticks
	_display.tick(); // out of the interact mode scope because of display error led timer
	if (isModeInteract()) {
		_dltransMain.tick();
		if (DisplayLayout* activeDL = getActiveDisplayLayout())
			activeDL->tick();
	}
}

void Application::makeMeasurement(bool storeData) {
	// Temperature
	{
		if (!_sensorTemp.measure()) {
			LOGLN(F("Couldn't start measuring temperature!"));
			return;
		}
		// TODO: refactor in a proper way
		TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement()); // TODO: please, no blocking function calls!!!
		if (!dataPtr) {
			LOGLN(F("Couldn't get measurement even after 1s!"));
			return;
		}

		if (storeData) {
			Storage::addMeasurementData(dataPtr->temp);
		}
		
		if (isModeInteract())
			_dLayouts[DisplayLayoutKeys::MAIN]->update(dataPtr);
		
		// LOG(F("Temperature: "));
		// LOGLN(dataPtr->temp);
	}
}
bool Application::sleep() {
	const SStrSleeping& sleeping = Storage::getSleeping();
	if (!Storage::setSleeping(millis() + sleeping.timeAwake, _mode))
		return false;
	if (isModeInteract()) { // TODO: should not happen here, rather by sleep-timer or by explicit button click
		_display->clearDisplay();
		_display->display();
	}
	setModeBackground(); // TODO: should clearing display be happening here?
	uint64_t period = _settings.getEntry<uint16_t>(ThSettings::Entries::PERIOD_CAPTURE) * 1e6;
	ESP.deepSleep(period, RF_DISABLED); // TODO: this is generally wrong as doesn't account for time passed in BI-mode
	return true;
}

bool Application::startBackgroundJob() {
	if (!Storage::cleanDatafile()) {
		DLOGLN("Couldn't start background job!");
		return false;
	}
	setInProgress(true);
	DLOGLN("Background job started!");
	return true;
}

bool Application::stopBackgroundJob() {
	setInProgress(false);
	activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE, true);
	DLOGLN("Background job stopped!");
	return Storage::removeSleeping();
}

void Application::activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style, bool force) {
	if (!isModeInteract() || dLayoutKey == DisplayLayoutKeys::NONE || (dLayoutKey == _dLayoutActiveKey && !force)) {
		return;
	}
	DisplayLayout* target = _dLayouts[dLayoutKey].get();

	if (style == DLTransitionStyle::NONE) {
		target->transitionEnterStarted();
		if (DisplayLayout* activeDL = getActiveDisplayLayout())
			activeDL->transitionLeaveStarted();
		target->transitionEnterFinished();
		if (DisplayLayout* activeDL = getActiveDisplayLayout())
			activeDL->transitionLeaveFinished();
	} else {
		Display::ScrollDir direction = Display::ScrollDir::LEFT;
		switch (style) {
			case DLTransitionStyle::AUTO: {
				int8_t keyDst = dLayoutKey - _dLayoutActiveKey;
				bool jump = abs(keyDst) > 1;
				direction = ((keyDst < 0 && jump) || (keyDst > 0 && !jump)) ? Display::ScrollDir::LEFT : Display::ScrollDir::RIGHT;
				break;
			}
			case DLTransitionStyle::RIGHT: {
				direction = Display::ScrollDir::RIGHT;
				break;
			}
		}
		_dltransMain.start(getActiveDisplayLayout(), target, direction);
	}
	_dLayoutActiveKey = dLayoutKey; // TODO: should be set only after transition is finished??
	
	// // Some flags can persist from previous layouts control handling, reset them
	// // TODO: better design, please!
	// // TODO: does this even work??
	// if (PushButton* btn = static_cast<PushButton*>(_inputs.getInput(HIChannel::BUTTON1)))
	// 	btn->reset();
	// if (PushButton* btn = static_cast<PushButton*>(_inputs.getInput(HIChannel::BUTTON2)))
	// 	btn->reset();
}

bool Application::initDisplayStuff() {
	// Init display at first place as this is a must-have in the user interaction mode
	if (!_display.init())
		return false;
	reportInitialized("Display initialized");
	_display->clearDisplay();
	
	// Order should follow the order in DisplayLayouts
	for (uint8_t i = 0; i < DisplayLayoutKeys::_COUNT; ++i)
		_dLayouts.push_back(nullptr);
	_dLayouts[DisplayLayoutKeys::WELCOME].reset(new DLayoutWelcome);
	_dLayouts[DisplayLayoutKeys::BACKGROUND_INTERRUPTED].reset(new DLayoutBackgroundInterrupted);
	_dLayouts[DisplayLayoutKeys::GRAPH].reset(new DLayoutGraph);
	// Main menu carousel layouts
	_dLayouts[DisplayLayoutKeys::MAIN].reset(new DLayoutMain);
	_dLayouts[DisplayLayoutKeys::MEASVIEWER].reset(new DLayoutMeasViewer);
	_dLayouts[DisplayLayoutKeys::SETTINGS].reset(new DLayoutSettings);

	for (auto& dlayout : _dLayouts)
		if (!dlayout->init(&_display, this, &_inputs))
			return false;
	reportInitialized("Display layouts initialized");

	if (!_dltransMain.init(&_display, 200, DLTransition::Interpolation::APOW3))
		return false;
	reportInitialized("Display transition initialized");

	return true;
}

bool Application::debugInitLFSExplorer() {
	delay(MODE_DETECTION_DELAY);
	if (!_inputs.tick(HIChannel::BUTTON1) && _inputs.tick(HIChannel::BUTTON2)) {
		DLOGLN(F("[DEBUG] LittleFS explorer mode! Run 'help' to check available commands."));
		_mode = Mode::_DEBUG_LITTLEFS_EXPLORER;
		if (_display.init()) {
			delay(1);
			_display->clearDisplay();
			_display->setTextColor(DISPLAY_WHITE);
			_display->setCursor(0, 0);
			_display->setTextSize(1);
			_display->print(F("LFSexplorer mode"));
			_display->display();
		}
		return true;
	}
	return false;
}