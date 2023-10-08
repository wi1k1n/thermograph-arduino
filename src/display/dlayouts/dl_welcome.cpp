#include "display/dlayouts/dl_welcome.h"
#include "application.h"
#include "logo.h"

bool DLayoutWelcome::init(Display* display, Application* app, HardwareInputs* inputs) {
	bool success = DisplayLayout::init(display, app, inputs);
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