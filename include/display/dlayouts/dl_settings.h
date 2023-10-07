#ifndef DL_SETTINGS_H__
#define DL_SETTINGS_H__

#include "display/display_layout.h"

class DLayoutSettings : public DisplayLayout {
	float _temp1;
	Timer _timerRandomPixel;
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void activate() override;
	void deactivate() override;
	void update(void* data) override;
	void tick() override;
};

#endif // DL_SETTINGS_H__