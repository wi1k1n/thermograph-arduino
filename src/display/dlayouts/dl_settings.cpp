#include "display/dlayouts/dl_settings.h"
#include "main.h"

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

	if (!_btn1->tick() && !_btn2->tick()) {
		return;
	}
	if (!_btn2->down() && _btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::GRAPH);
		return;
	}
	if (!_btn1->down() && _btn2->click()) {
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