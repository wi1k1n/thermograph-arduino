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
static void reportInitialized(uint8_t idx, const String& str) {
	DLOGLN("(" + String(idx) + "/" + String(INITIALIZATION_STEPS) + ") " + str);
}

bool Application::setup() {
	_UART_.begin(115200);
	DLOGLN(F("Welcome to Thermograph v2"));

	if (!ThFS::init())
		return false;
	reportInitialized(1, "ThFS initialized");

	{
		uint8_t btnPin = INTERACT_PUSHBUTTON_1_PIN;
		if (!_inputs.init(HIChannel::BUTTON1, &btnPin))
			return false;
		btnPin = INTERACT_PUSHBUTTON_2_PIN;
		if (!_inputs.init(HIChannel::BUTTON2, &btnPin))
			return false;
		reportInitialized(2, "Buttons initialized");
	}

#ifdef TDEBUG // Hold btn2 to load Serial LittleFS explorer code
	if (debugInitLFSExplorer())
		return true;
#endif

	if (!Storage::init())
		return false;
	reportInitialized(3, "Storage manager initialized");

	if (!_settings.init())
		return false;
	reportInitialized(4, "Settings manager initialized");
	
	// If not sleeping -> INTERACT mode
	const SStrSleeping& sleepingEntry = Storage::getSleeping(true);
	bool isSleeping = sleepingEntry.timeAwake > 0;
	if (!isSleeping) {
		_mode = Mode::INTERACT;
#ifdef TDEBUG // Hold btn1 to force app to load in BI mode
		delay(MODE_DETECTION_DELAY);
		if (_inputs.tick(HIChannel::BUTTON1) && !_inputs.tick(HIChannel::BUTTON2)) {
			DLOGLN(F("[DEBUG] FORCED TO ENTER BACKGROUND_INTERRUPTED MODE!"));
			setModeBackgroundInterrupted();
		}
#endif
	} else {
		DLOG(F("isSleeping == true: timeAwake = "));
		LOGLN(sleepingEntry.timeAwake);
		// Decide what mode are we loading in
		delay(MODE_DETECTION_DELAY);
		if (_inputs.tick(HIChannel::BUTTON1) && _inputs.tick(HIChannel::BUTTON2)) {
			DLOG(F("Button1 and Button2: pressed"));
			setModeBackgroundInterrupted();
		}
	}
	DLOG(F("Mode: "));
	LOGLN(_mode);

	if (isInteractionAvailable())
		if (!initDisplayStuff())
			return false;

	if (!_sensorTemp.init())
		return false;
	reportInitialized(8, "Sensor temp initialized");
	
	if (isModeInteract()) {
		activateDisplayLayout(DisplayLayoutKeys::WELCOME, DLTransitionStyle::NONE);
	} else if (isModeBackgroundInterrupted()) {
		activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE);
		// activateDisplayLayout(DisplayLayoutKeys::BACKGROUND_INTERRUPTED, DLTransitionStyle::NONE); // TODO: remove this display layout at all!
	}

	return true;
}

void Application::loop() {
#ifdef TDEBUG
	if (_mode == Mode::_DEBUG_LITTLEFS_EXPLORER)
		return DEBUG::LittleFSExplorer("");
#endif
	_display.tick(); // out of the interact mode scope because of display error led timer
	if (isInteractionAvailable()) {
		_dltransMain.tick();
		getActiveDisplayLayout()->tick();
	}
}

void Application::makeMeasurement() {
	// TODO: refactor in a proper way
	if (_sensorTemp.measure()) {
		TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement());
		if (dataPtr) {
			if (isInteractionAvailable()) {
				_dLayouts[DisplayLayoutKeys::MAIN]->update(dataPtr);
			}
			// LOG(F("Temperature: "));
			LOGLN(dataPtr->temp);
		} else {
			LOGLN(F("Couldn't get measurement even after 1s!"));
		}
	} else {
		LOGLN(F("Couldn't start measuring temperature!"));
	}
}
bool Application::startBackgroundJob() {
	const SStrSleeping& sleeping = Storage::getSleeping();
	if (!Storage::setSleeping(millis() + sleeping.timeAwake, _mode))
		return false;
	if (isModeInteract() || isModeBackgroundInterrupted()) {
		_display->clearDisplay();
		_display->display();
	}
	ESP.deepSleep(5e6);
	return true;
}

bool Application::stopBackgroundJob() {
	setModeInteract();
	activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE, true);
	return Storage::removeSleeping();
}

void Application::activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style, bool force) {
	if (!isInteractionAvailable() || dLayoutKey == DisplayLayoutKeys::NONE || (dLayoutKey == _dLayoutActiveKey && !force)) {
		return;
	}
	DisplayLayout* target = _dLayouts[dLayoutKey].get();

	if (style == DLTransitionStyle::NONE) {
		target->activate();
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
	_dLayoutActiveKey = dLayoutKey;
	
	// Some flags can persist from previous layouts control handling, reset them
	// TODO: better design, please!
	if (PushButton* btn = static_cast<PushButton*>(_inputs.getInput(HIChannel::BUTTON1)))
		btn->reset();
	if (PushButton* btn = static_cast<PushButton*>(_inputs.getInput(HIChannel::BUTTON2)))
		btn->reset();
}

bool Application::initDisplayStuff() {
	// Init display at first place as this is a must-have in the user interaction mode
	if (!_display.init())
		return false;
	reportInitialized(5, "Display initialized");
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
	reportInitialized(6, "Display layouts initialized");

	if (!_dltransMain.init(&_display, 200, DLTransition::Interpolation::APOW3))
		return false;
	reportInitialized(7, "Display transition initialized");

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