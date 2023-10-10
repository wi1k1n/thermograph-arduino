#ifndef DL_MAIN_H__
#define DL_MAIN_H__

#include "display/display_layout.h"

class Application;

/// @brief Main DL implementation. Shows real-time sensor values and allows to start BG task.
class DLayoutMain : public DisplayLayout {
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void activate() override;
	void deactivate() override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;

private:
	void updateButtonTitle(bool doDisplay = false);

private:
	float _temp1 = -1;
	DLButton _gBtnMain;
	LED _debugLED;
};

#endif // DL_MAIN_H__