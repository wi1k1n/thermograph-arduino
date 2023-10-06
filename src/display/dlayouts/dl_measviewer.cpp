#include "display/dlayouts/dl_measviewer.h"
#include "main.h"

bool DLayoutMeasViewer::init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) {
	bool success = DisplayLayout::init(display, app, btn1, btn2);
	return success && _timerRandomPixel.init(120, Timer::MODE::PERIOD);
}
void DLayoutMeasViewer::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutMeasViewer::activate() {
	DisplayLayout::activate();
	_timerRandomPixel.start();
}
void DLayoutMeasViewer::deactivate() {
	DisplayLayout::deactivate();
	_timerRandomPixel.stop();
}
void DLayoutMeasViewer::tick() {
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
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (!_btn1->down() && _btn2->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
		return;
	}
}
void DLayoutMeasViewer::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Measurement Viewer"));

	if (doDisplay) {
		display()->display();
	}
}