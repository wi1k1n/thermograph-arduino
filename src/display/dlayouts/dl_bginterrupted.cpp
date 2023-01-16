#include "display/dlayouts/dl_bginterrupted.h"

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