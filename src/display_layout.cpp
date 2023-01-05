#include "display_layout.h"
#include "logo.h"
#include "main.h"

bool DLGUI::init(Display* display) {
    if (!display)
        return false;
    _display = display;
    return true;
}

bool DLButton::init(Display* display, UPoint pos, UPoint size, const String& title) {
    if (!DLGUI::init(display))
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
void DLButton::tick() {

}
void DLButton::draw(bool doDisplay, bool clearFirst) {
	DLGUI::draw(doDisplay, clearFirst);
	// DLOGLN();
    if (clearFirst)
	    display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, DISPLAY_BLACK);
    
    display()->drawRect(_pos.x, _pos.y, _size.x, _size.y, DISPLAY_WHITE);
    
	display()->setTextColor(DISPLAY_WHITE);
    display()->setCursor(_pos.x + _titleOffset.x, _pos.y + _titleOffset.y);
	display()->setTextSize(_textSize);
	display()->print(_title);
    
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
	_btn1->reset();
	_btn2->reset();
	if (_timer.tick()) {
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

    _gBtn.draw(doDisplay, true);
    
	if (doDisplay) {
		display()->display();
	}
}
void DLayoutBackgroundInterrupted::tick() {
	DisplayLayout::tick();
	_btn1->reset();
	_btn2->reset();
}

void DLayoutMain::update(void* data) {
	DisplayLayout::draw(data);
	TempSensorData* tempData = static_cast<TempSensorData*>(data);
	_temp1 = tempData->temp;
}
void DLayoutMain::tick() {
	DisplayLayout::tick();
	if (_btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
		return;
	}
	if (_btn2->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
		return;
	}
}
void DLayoutMain::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();

	display()->setTextColor(DISPLAY_WHITE);

	char buffer[16];
	dtostrf(_temp1, 6, 2, buffer);
	
	display()->setCursor(0, DISPLAY_LAYOUT_PADDING_TOP);
	display()->setTextSize(3);
	display()->print(buffer);

	display()->setCursor(display()->getCursorX(), display()->getCursorY() - 4);
	display()->setTextSize(2);
	display()->print(F("o"));
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Thermograph v2"));

	if (doDisplay) {
		display()->display();
	}
}


void DLayoutGraph::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutGraph::tick() {
	DisplayLayout::tick();
	if (_btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (_btn2->click()) {
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

	if (_btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
		return;
	}
	if (_btn2->click()) {
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