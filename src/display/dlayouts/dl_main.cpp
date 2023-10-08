#include "display/dlayouts/dl_main.h"
#include "application.h"

void DLayoutMain::drawGButtons(bool doDisplay) {
    _gBtnStart.draw(doDisplay);
    _gBtnResume.draw(doDisplay);
    _gBtnStop.draw(doDisplay);
}

void DLayoutMain::adjustGButtonsModeInteract() {
	_gBtnResume.setFocused(false);
	_gBtnStop.setFocused(false);
	_gBtnStart.setFocused(false);

	_gBtnResume.setVisible(false);
	_gBtnStop.setVisible(false);
	_gBtnStart.setVisible(true);
}

void DLayoutMain::adjustGButtonsModeBGInterrupted() {
	_gBtnResume.setFocused(false);
	_gBtnStop.setFocused(false);
	_gBtnStart.setFocused(false);

	_gBtnStart.setVisible(false);
	_gBtnResume.setVisible(true);
	_gBtnStop.setVisible(true);
}

bool DLayoutMain::init(Display* display, Application* app, HardwareInputs* inputs) {
	const bool success = DisplayLayout::init(display, app, inputs);
	_debugLED.init(LED_BUILTIN);
    const uint8_t bWidth = 42;
    const uint8_t bHeight = 16;
	const uint8_t bPadding = 10;
    const UPoint bSize{ bWidth, bHeight };
	const uint8_t dWidth = _display->rawWidth();
    const uint16_t bY = _display->rawHeight() - bHeight;
	return success
		&& _timerMeasure.init(DISPLAY_LAYOUT_MAIN_MEASUREMENT_PERIOD)
        && _gBtnStart.init(display, {(dWidth - bWidth) / 2, bY}, bSize, "Start")
        && _gBtnResume.init(display, {(dWidth - 2 * bWidth - bPadding) / 2, bY}, bSize, "Resume", 0)
        && _gBtnStop.init(display, {(dWidth - 2 * bWidth - bPadding) / 2 + bWidth + bPadding, bY}, bSize, "Stop", 0);
}

void DLayoutMain::activate() {
    if (_app->isModeBackgroundInterrupted()) {
		DLOGLN(F("DLMain activated in BI mode"));
		adjustGButtonsModeBGInterrupted();
    } else {
		DLOGLN(F("DLMain activated in I mode"));
		adjustGButtonsModeInteract();
    }
	_timerMeasure.start();
    DisplayLayout::activate();
}

void DLayoutMain::deactivate() {
	_timerMeasure.stop();
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

	if (_timerMeasure.tick()) {
		_app->makeMeasurement();
	}
	
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

	// Only if btn2 is NOT pressed
	if (!btn2->down()) {
		// LOGLN("!2state");
		if (_app->isModeInteract()) {
			// LOGLN("Mode I");
			if (_gBtnStart.isPressed()) {
				// LOGLN("gbtnStart focused");
				if (btn1->release()) {
					// LOGLN("1release -> change mode BI");
					// _app->setModeBackgroundInterrupted();
					// activate(); // TODO: turn off directly
					if (!_app->startBackgroundJob())
						DLOGLN("Couldn't start background job!");
				}
			} else {
				// LOGLN("gbtnStart !focused");
				if (btn1->click()) {
					// LOGLN(F("1click -> change menu to Settings"));
					_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
				}
				if (btn1->held()) {
					// LOGLN("1held -> gbtnStart set focused");
					_gBtnStart.setPressed(true, true, true);
				}
			}
		} else if (_app->isModeBackgroundInterrupted()) {
			// LOGLN("Mode BI");
			if (btn1->click()) {
				// LOGLN(F("1click -> change menu to Settings"));
				_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
			}
		}
	}
	// Only if btn1 NOT pressed
	if (!btn1->down()) {
		// LOGLN("!1state");
		if (_app->isModeInteract()) {
			// LOGLN("Mode I");
			if (btn2->click()) {
				// LOGLN(F("2click -> change menu to Graph"));
				_app->activateDisplayLayout(DisplayLayoutKeys::MEASVIEWER);
			}
		} else if (_app->isModeBackgroundInterrupted()) {
			// LOGLN("Mode BI");
			if (btn2->click()) {
				// LOGLN(F("2click -> change mode to I"));
				// _app->setModeInteract();
				// activate(); // TODO: turn off directly
				_app->stopBackgroundJob();
			}
		}
	}
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
	
	drawGButtons();

	if (doDisplay) {
		display()->display();
	}
}