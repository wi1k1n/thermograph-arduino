#include "display_layout.h"
#include "logo.h"
#include "main.h"

bool DLGUI::init(Display* display, bool visible, bool focused) {
    if (!display)
        return false;
    _display = display;
    setVisible(visible);
    setFocused(focused);
    return true;
}

void DLGUI::setVisible(bool visible, bool reDraw, bool doDisplay) {
	_visible = visible;
	if (reDraw) {
		if (_visible) {
			draw(doDisplay);
		} else {
			clear(doDisplay);
		}
	}
}

void DLGUI::setFocused(bool focused, bool reDraw, bool doDisplay) {
	_focused = focused;
	if (reDraw) {
		draw(doDisplay);
	}
}

///////////////////////////////

bool DLButton::init(Display* display, UPoint pos, UPoint size, const String& title, bool visible, bool focused) {
    if (!DLGUI::init(display, visible, focused))
        return false;
    _pos = pos;
    _size = size;
    _title = title;

    if (size.y >= DISPLAY_FONT_HEIGHT * 2 && size.x >= title.length() * DISPLAY_FONT_WIDTH * 2) {
        _textSize = 2;
        _borderThickness = 2;
    }
    
    uint16_t titleWidth = title.length() * _textSize * DISPLAY_FONT_WIDTH - _textSize;
    uint8_t titleHeight = _textSize * DISPLAY_FONT_HEIGHT - _textSize;
    _titleOffset.x = max((_size.x - titleWidth) / 2, 0);
    _titleOffset.y = max((_size.y - titleHeight) / 2, 0);

    return true;
}

void DLButton::draw(bool doDisplay, bool clearFirst) {
	DLGUI::draw(doDisplay, clearFirst);
    if (!isVisible()) {
        return;
    }

	// DLOGLN();
	uint8_t clrBG = _focused ? DISPLAY_WHITE : DISPLAY_BLACK;
	uint8_t clrFG = _focused ? DISPLAY_BLACK : DISPLAY_WHITE;

	if (clearFirst && !_focused) {
		clear(false);
	}
    if (_focused) {
	    display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, clrBG);
	} else {
    	display()->drawRect(_pos.x, _pos.y, _size.x, _size.y, clrFG);
	}
    
	display()->setTextColor(clrFG);
    display()->setCursor(_pos.x + _titleOffset.x, _pos.y + _titleOffset.y);
	display()->setTextSize(_textSize);
	display()->print(_title);
    
	if (doDisplay) {
		display()->display();
	}
}

void DLButton::clear(bool doDisplay) {
	display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, DISPLAY_BLACK);
	if (doDisplay) {
		display()->display();
	}
}

/////////////////////////////////

bool DisplayLayout::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	if (!display || !app || !btn1 || !btn2)
		return false;
	_display = display;
	_app = app;
	_btn1 = btn1;
	_btn2 = btn2;
	return true;
}

bool DLayoutWelcome::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	bool success = DisplayLayout::init(display, app, btn1, btn2);
	return success && _timer.init(DISPLAY_LAYOUT_LOGO_DELAY, Timer::MODE::TIMER);
}
void DLayoutWelcome::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutWelcome::activate() {
	DisplayLayout::activate();
	_timer.start();
}
void DLayoutWelcome::deactivate() {
	DisplayLayout::deactivate();
	_timer.stop();
}
void DLayoutWelcome::tick() {
	DisplayLayout::tick();
	if (_timer.tick()) {
		// _btn1->reset();
		// _btn2->reset();
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE);
	}
}
void DLayoutWelcome::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();

	display()->drawBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, DISPLAY_WHITE);

	if (doDisplay) {
		display()->display();
	}
}

bool DLayoutBackgroundInterrupted::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	bool success = DisplayLayout::init(display, app, btn1, btn2);
	return success && _gBtn.init(display, {32, 32}, {42, 16}, "Pushme");
}
void DLayoutBackgroundInterrupted::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	DLOGLN();
	display()->clearDisplay();

	display()->setTextColor(DISPLAY_WHITE);
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Interrupted!"));

    _gBtn.draw();
    
	if (doDisplay) {
		display()->display();
	}
}
void DLayoutBackgroundInterrupted::tick() {
	DisplayLayout::tick();
	// bool tickBtn1 = _btn1->tick();
	// bool tickBtn2 = _btn2->tick();
}

//////////////////////////

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

bool DLayoutMain::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	const bool success = DisplayLayout::init(display, app, btn1, btn2);
	_debugLED.init(LED_BUILTIN);
    const uint8_t bWidth = 42;
    const uint8_t bHeight = 16;
	const uint8_t bPadding = 10;
    const UPoint bSize{ bWidth, bHeight };
	const uint8_t dWidth = _display->rawWidth();
    const uint16_t bY = _display->rawHeight() - bHeight;
	return success
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
    DisplayLayout::activate();
}
void DLayoutMain::update(void* data) {
	DisplayLayout::draw(data);
	TempSensorData* tempData = static_cast<TempSensorData*>(data);
	_temp1 = tempData->temp;
}
void DLayoutMain::tick() {
	DisplayLayout::tick();
	bool tickBtn1 = _btn1->tick();
	bool tickBtn2 = _btn2->tick();
	// Don't waste time if nothing to process
	if (!tickBtn1 && !tickBtn2) {
		_debugLED.on();
		return;
	}
	_debugLED.off();

	if (_btn1->state()) {
		if (!_btn2->state()) {
			LOGLN(F("tick: 1 & !2"));
			if (_app->isModeBackgroundInterrupted()) { // in BI mode
				LOG(F("Focus: "));
				LOG(_gBtnStart.isFocused());
				LOG(_gBtnStop.isFocused());
				LOGLN(_gBtnResume.isFocused());
				if (!_gBtnResume.isFocused() && !_gBtnStop.isFocused()) {
					if (_btn1->click()) { // just scroll menu
						_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
					}
				} else {
				}
			} else {
				if (_gBtnStart.isFocused()) { // already in focus
					LOGLN(F("Start-inFocus"));
					if (_btn1->release()) {
						LOGLN(F("1-release -> start unfocused, mode changed to BI"));
						// _gBtnStart.setFocused(false);
						_app->setModeBackgroundInterrupted();
						activate(); // TODO: turn off directly
					}
				} else { // just started pressing button
					LOGLN(F("Start-!focused"));
					if (_btn1->hold()) { // enter focusing 'start' button
						LOGLN(F("1-hold -> start focused"));
						_gBtnStart.setFocused(true, true, true);
					}
					if (_btn1->click()) { // just scroll menu
						LOGLN(F("Menu change -> Settings"));
						_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
					}
				}
			}
		} else {
			if (_btn1->hold() && _btn2->press()) {
				LOGLN(F("1-hold & 2-press"));
				_btn1->reset();
				_btn2->reset();
			}
		}
	}
	if (_btn2->state() && !_btn1->state()) {
		LOGLN(F("tick: 2 & !1"));
		if (_app->isModeBackgroundInterrupted()) {
			LOGLN(F("DEBUG: back to interact"));
			if (_btn2->click()) {
				_app->setModeInteract();
				activate(); // TODO: turn off directly
			}
		} else if (_btn2->click()) {
			LOGLN(F("Menu change -> Graph"));
			_app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
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
	
	display()->setCursor(0, DISPLAY_LAYOUT_PADDING_TOP);
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


void DLayoutGraph::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutGraph::tick() {
	DisplayLayout::tick();
	bool tickBtn1 = _btn1->tick();
	bool tickBtn2 = _btn2->tick();
	if (tickBtn1 && _btn1->state() && _btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (tickBtn2 && _btn2->state() && _btn2->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
		return;
	}
}
void DLayoutGraph::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Graph Layout"));

	if (doDisplay) {
		display()->display();
	}
}

bool DLayoutSettings::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	bool success = DisplayLayout::init(display, app, btn1, btn2);
	return success && _timerRandomPixel.init(120, Timer::MODE::PERIOD);
}
void DLayoutSettings::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutSettings::activate() {
	DisplayLayout::activate();
	_timerRandomPixel.start();
}
void DLayoutSettings::deactivate() {
	DisplayLayout::deactivate();
	_timerRandomPixel.stop();
}
void DLayoutSettings::tick() {
	DisplayLayout::tick();
	if (_timerRandomPixel.tick()) {
		int16_t x = random(display()->width()),
				y = random(display()->height());
		uint16_t clr = display()->getPixel(x, y) ? DISPLAY_BLACK : DISPLAY_WHITE;
		display()->drawPixel(x, y, clr);
		display()->display();
	}

	_btn1->tick();
	_btn2->tick();
	if (_btn1->state() && _btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
		return;
	}
	if (_btn2->state() && _btn2->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
}
void DLayoutSettings::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Settings Layout"));

	if (doDisplay) {
		display()->display();
	}
}