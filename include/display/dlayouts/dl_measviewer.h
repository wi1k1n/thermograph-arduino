#ifndef DL_MEASVIEWER_H__
#define DL_MEASVIEWER_H__

#include "display/display_layout.h"

class Application;

class DLayoutMeasViewer : public DisplayLayout {
	float _temp1;
	Timer _timerRandomPixel;
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
	
	void transitionEnterFinished() override;
	void transitionLeaveStarted() override;
};

#endif // DL_MEASVIEWER_H__