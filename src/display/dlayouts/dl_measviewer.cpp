#include "display/dlayouts/dl_measviewer.h"
#include "application.h"

bool DLayoutMeasViewer::init(Display* display, Application* app, HardwareInputs* inputs) {
	bool success = DisplayLayout::init(display, app, inputs);
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
	
	PushButton* btn1 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON1));
	PushButton* btn2 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON2));
	if (!btn1 || !btn2)
		return;

	if (!btn1->tick() && !btn2->tick()) {
		return;
	}
	if (!btn2->down() && btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (!btn1->down() && btn2->click()) {
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