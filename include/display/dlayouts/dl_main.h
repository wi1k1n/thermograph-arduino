#ifndef DL_MAIN_H__
#define DL_MAIN_H__

#include "display/display_layout.h"

class Application;

/// @brief Main DL implementation. Shows real-time sensor values and allows to start BG task.
class DLayoutMain : public DisplayLayout {
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;

	void transitionEnterStarted() override;
	void transitionEnterFinished() override;
	void transitionLeaveStarted() override;

private:
	void updateButtons(bool doDisplay = false);

private:
	float _temp1 = 0;

	DLButton _gBtnMain;
	DLButton _gBtnSleep;
	LED _debugLED;
};

#endif // DL_MAIN_H__