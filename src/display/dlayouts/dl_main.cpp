#include "display/dlayouts/dl_main.h"
#include "application.h"

void DLayoutMain::updateButtonTitle(bool doDisplay) {
	String title = _app->isModeBackgroundInterrupted() ? "Stop" : "Start";
	_gBtnMain.setTitle(title);
}

bool DLayoutMain::init(Display* display, Application* app, HardwareInputs* inputs) {
	bool success = DisplayLayout::init(display, app, inputs);
	success &= _debugLED.init(LED_BUILTIN);

    const uint8_t bWidth = 42;
    const uint8_t bHeight = 15;
	const uint8_t bPadding = 10;
    const UPoint bSize{ bWidth, bHeight };
	const uint8_t dWidth = _display->rawWidth();
    const uint16_t bY = _display->rawHeight() - bHeight;
	
	success &= _gBtnMain.init(display, {(dWidth - bWidth) / 2, bY}, bSize, "");
	updateButtonTitle();

	return success;
}

void DLayoutMain::activate() {
    DisplayLayout::activate();
}

void DLayoutMain::deactivate() {
    DisplayLayout::deactivate();
}

void DLayoutMain::update(void* data) {
	DisplayLayout::draw(data);

	TempSensorData* tempData = static_cast<TempSensorData*>(data);
	_temp1 = tempData->temp;

	draw();
}

void DLayoutMain::tick() {
	DisplayLayout::tick();
	
	PushButton* btn1 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON1));
	PushButton* btn2 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON2));
	if (!btn1 || !btn2)
		return;

	// Don't waste time if no user control to process
	if (!btn1->tick() && !btn2->tick()) {
		// _debugLED.off();
		return;
	}
	// _debugLED.on();

	if (!btn2->down() && btn1->click()) { // Navigate to the left
		_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
		return;
	}
	if (!btn1->down() && btn2->click()) { // Navigate to the right
		_app->activateDisplayLayout(DisplayLayoutKeys::MEASVIEWER);
		return;
	}
	
	if (_app->isModeBackgroundInterrupted()) { // Measurements in progress
		if (!btn2->down() && btn1->held()) { // Started pressing "Stop"
			_gBtnMain.setPressed(true);
		}
		if (btn2->down() && _gBtnMain.isPressed()) { // Pressing "Stop" interrupted
			_gBtnMain.setPressed(false);
		}
		if (!btn2->down() && btn1->release() && _gBtnMain.isPressed()) { // Finished pressing "Stop"
			_app->setModeInteract();
			_gBtnMain.setPressed(false);
			updateButtonTitle();
			_app->stopTimerMeasurement();
		}
		draw();
		return;
	} else if (_app->isModeInteract()) { // No ongoing measurements
		if (!btn2->down() && btn1->held()) { // Started pressing "Start"
			_gBtnMain.setPressed(true);
		}
		if (btn2->down() && _gBtnMain.isPressed()) { // Pressing "Start" interrupted
			_gBtnMain.setPressed(false);
		}
		if (!btn2->down() && btn1->release() && _gBtnMain.isPressed()) { // Finished pressing "Start"
			_app->setModeBackgroundInterrupted();
			_gBtnMain.setPressed(false);
			updateButtonTitle();
			_app->startTimerMeasurement();
		}
		draw();
		return;
	}

	// // Only if btn2 is NOT pressed
	// if (!btn2->down()) {
	// 	// LOGLN("!2state");
	// 	if (_app->isModeInteract()) {
	// 		// LOGLN("Mode I");
	// 		if (_gBtnStart.isPressed()) {
	// 			// LOGLN("gbtnStart focused");
	// 			if (btn1->release()) {
	// 				// LOGLN("1release -> change mode BI");
	// 				// _app->setModeBackgroundInterrupted();
	// 				// activate(); // TODO: turn off directly
	// 				if (!_app->startBackgroundJob())
	// 					DLOGLN("Couldn't start background job!");
	// 			}
	// 		} else {
	// 			// LOGLN("gbtnStart !focused");
	// 			if (btn1->click()) {
	// 				// LOGLN(F("1click -> change menu to Settings"));
	// 				_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
	// 			}
	// 			if (btn1->held()) {
	// 				// LOGLN("1held -> gbtnStart set focused");
	// 				_gBtnStart.setPressed(true, true, true);
	// 			}
	// 		}
	// 	} else if (_app->isModeBackgroundInterrupted()) {
	// 		// LOGLN("Mode BI");
	// 		if (btn1->click()) {
	// 			// LOGLN(F("1click -> change menu to Settings"));
	// 			_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
	// 		}
	// 	}
	// }
	// // Only if btn1 NOT pressed
	// if (!btn1->down()) {
	// 	// LOGLN("!1state");
	// 	if (_app->isModeInteract()) {
	// 		// LOGLN("Mode I");
	// 		if (btn2->click()) {
	// 			// LOGLN(F("2click -> change menu to Graph"));
	// 			_app->activateDisplayLayout(DisplayLayoutKeys::MEASVIEWER);
	// 		}
	// 	} else if (_app->isModeBackgroundInterrupted()) {
	// 		// LOGLN("Mode BI");
	// 		if (btn2->click()) {
	// 			// LOGLN(F("2click -> change mode to I"));
	// 			// _app->setModeInteract();
	// 			// activate(); // TODO: turn off directly
	// 			_app->stopBackgroundJob();
	// 		}
	// 	}
	// }
}

void DLayoutMain::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();

	display()->setTextColor(DISPLAY_WHITE);

	char buffer[16];
	dtostrf(_temp1, 6, 1, buffer);
	
	display()->setCursor(0, 16);
	display()->setTextSize(3);
	display()->print(buffer);

	display()->setCursor(display()->getCursorX(), display()->getCursorY() - 4);
	display()->setTextSize(2);
	display()->print(F("o"));
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Thermograph v2"));
	
	_gBtnMain.draw();

	if (doDisplay) {
		display()->display();
	}
}