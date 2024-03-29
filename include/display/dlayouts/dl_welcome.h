#ifndef DL_WELCOME_H__
#define DL_WELCOME_H__

#include "display/display_layout.h"

class Application;

class DLayoutWelcome : public DisplayLayout {
	Timer _timer;
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
	
	void transitionEnterFinished() override;
	void transitionLeaveStarted() override;
};

#endif // DL_WELCOME_H__