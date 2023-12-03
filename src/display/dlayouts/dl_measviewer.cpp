#include "display/dlayouts/dl_measviewer.h"
#include "application.h"
#include "filesystem.h"

bool DLayoutMeasViewer::init(Display* display, Application* app, HardwareInputs* inputs) {
	bool success = DisplayLayout::init(display, app, inputs);
	return success && _timerRandomPixel.init(120, Timer::MODE::PERIOD);
}
void DLayoutMeasViewer::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutMeasViewer::transitionEnterFinished() {
	DisplayLayout::transitionEnterFinished();
	_timerRandomPixel.start();
	
	Storage::readData(_data);
	// LOG("Data:"); for (auto d : _data) { LOG(" "); LOG(d); } LOGLN("");
	draw();
}
void DLayoutMeasViewer::transitionLeaveStarted() {
	DisplayLayout::transitionLeaveStarted();
	_timerRandomPixel.stop();

	_data.clear();
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

	display()->setCursor(0, 16);
	display()->setTextSize(1);
	for (const auto& d : _data) {
		display()->print((int)d);
		display()->print(F(" "));
		// LOG(" "); LOG(d);
	}

	if (doDisplay) {
		display()->display();
	}
}