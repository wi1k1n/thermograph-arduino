#include "dlwelcome.h"
#include "configuration.h"

#include "display/software/display_layouts/logo.h"

void DLWelcome::update(void* data) {
	DisplayLayoutInterface::update(data);
}
void DLWelcome::transitionEnterFinished() {
	DisplayLayoutInterface::transitionEnterFinished();
	// _timer.start();
}
void DLWelcome::transitionLeaveStarted() {
	DisplayLayoutInterface::transitionLeaveStarted();
	// _timer.stop();
}
void DLWelcome::tick() {
	DisplayLayoutInterface::tick();
	// if (_timer.tick()) {
	// 	// _btn1->reset();
	// 	// _btn2->reset();
	// 	_app->activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE);
	// }
}
void DLWelcome::draw() {
	DisplayLayoutInterface::draw();
	ASSERTPTR(_display);

	_display->drawBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, 0xFFFF);

	_display->display();

	// // DLOGLN();
	// display()->clearDisplay();

	// display()->drawBitmap(0, 0, static_cast<const uint8_t*>(logoData), LOGO_WIDTH, LOGO_HEIGHT, DISPLAY_WHITE);

	// if (doDisplay) {
	// 	display()->display();
	// }
}