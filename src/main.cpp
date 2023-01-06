#include "main.h"
#include "filesystem.h"

void Application::measureTemperature() {
	// TODO: refactor in a proper way
	if (_sensorTemp.measure()) {
		TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement());
		if (dataPtr) {
			if (isInteractionAvailable()) {
				_dLayouts[DisplayLayoutKeys::MAIN]->update(dataPtr);
			}
			LOG(F("Temperature: "));
			LOGLN(dataPtr->temp);
		} else {
			LOGLN(F("Couldn't get measurement even after 1s!"));
		}
	} else {
		LOGLN(F("Couldn't start measuring temperature!"));
	}
}

bool Application::setup() {
	// TODO: Serial is only for debugging purposes for now
	#ifdef TDEBUG
		Serial.begin(115200);
		delay(1);
		Serial.println();
	#endif
	DLOGLN(F("Welcome to Thermograph v2"));

	if (!Storage::init()) {
		return false;
	}
	DLOGLN(F("Storage manager initialized"));

	if (!_btn1.init(INTERACT_PUSHBUTTON_1_PIN))
		return false;
	if (!_btn2.init(INTERACT_PUSHBUTTON_2_PIN))
		return false;
	DLOGLN(F("Buttons initialized"));
	
	// If not sleeping -> INTERACT mode
	if (!Storage::isSleeping()) {
		_mode = Mode::INTERACT;
#ifdef TDEBUG
		delay(MODE_DETECTION_DELAY);
		if (_btn1.tick() && !_btn2.tick()) {
			DLOGLN(F("[DEBUG] FORCED TO ENTER BACKGROUND_INTERRUPTED MODE!"));
			setModeBackgroundInterrupted();
		}
#endif
	} else {
		DLOG(F("Storage manager: isSleeping() == true"));
		// Decide what mode are we loading in
		delay(MODE_DETECTION_DELAY);
		if (_btn1.tick() && _btn2.tick()) {
			DLOG(F("Button1 and Button2: pressed"));
			setModeBackgroundInterrupted();
		}
	}
	DLOG(F("Mode: "));
	LOGLN(_mode);

	if (isInteractionAvailable()) {
		// Init display at first place as this is must have in user interaction mode
		if (!_display.init(&Wire, 0, 0x3C))
			return false;
		DLOGLN(F("Display initialized"));
		_display->clearDisplay();
		
		// Order should follow the order in DisplayLayouts
		for (uint8_t i = 0; i < DisplayLayoutKeys::_COUNT; ++i) _dLayouts.push_back(nullptr);
		_dLayouts[DisplayLayoutKeys::WELCOME].reset(new DLayoutWelcome);
		_dLayouts[DisplayLayoutKeys::BACKGROUND_INTERRUPTED].reset(new DLayoutBackgroundInterrupted);
		// // Menu layouts
		_dLayouts[DisplayLayoutKeys::MAIN].reset(new DLayoutMain);
		_dLayouts[DisplayLayoutKeys::GRAPH].reset(new DLayoutGraph);
		_dLayouts[DisplayLayoutKeys::SETTINGS].reset(new DLayoutSettings);

		for (auto& dlayout : _dLayouts) {
			if (!dlayout->init(&_display, this, &_btn1, &_btn2))
				return false;
		}
		DLOGLN(F("Display layouts initialized"));

		if (!_dltransMain.init(&_display, 200, DLTransition::Interpolation::APOW3))
			return false;
		DLOGLN(F("Display transition initialized"));
	}

	if (!_sensorTemp.init())
		return false;
	DLOGLN(F("Sensor temp initialized"));
	measureTemperature();
	
	if (isModeInteract()) {
		activateDisplayLayout(DisplayLayoutKeys::WELCOME, DLTransitionStyle::NONE);
	} else if (isModeBackgroundInterrupted()) {
		activateDisplayLayout(DisplayLayoutKeys::BACKGROUND_INTERRUPTED, DLTransitionStyle::NONE);
	}

	return true;
}

void Application::loop() {
	_display.tick();
	// _btn1.tick();
	// _btn2.tick();

	if (isInteractionAvailable()) {
		_dltransMain.tick();
		getActiveDisplayLayout()->tick();
	}
}

void Application::activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style) {
	if (!isInteractionAvailable() || dLayoutKey == DisplayLayoutKeys::NONE || dLayoutKey == _dLayoutActiveKey) {
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
}

/////////////////////////

Application app;
// void setup() {
// 	const bool setupSucceeded = app.setup();
// 	DLOGLN(setupSucceeded);
// }
// void loop() {
// 	app.loop();
// }


// //The setup function is called once at startup of the sketch
// void setup() {
//   Serial.begin(115200);
//   while(!Serial) { }
//   Serial.println();
//   Serial.println("Start device in normal mode!");
 
//   delay(5000);
//   // Wait for serial to initialize.
//   while(!Serial) { }
 
//   // Deep sleep mode for 10 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
//   Serial.println("I'm awake, but I'm going into deep sleep mode for 10 seconds");
//   ESP.deepSleep(10e6);
// }
 
// void loop() {
// }

void setup() {

}
void loop() {
	
}