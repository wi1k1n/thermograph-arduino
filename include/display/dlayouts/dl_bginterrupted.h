#ifndef DL_BGINTERRUPTED_H__
#define DL_BGINTERRUPTED_H__

#include "display/display_layout.h"

class DLayoutBackgroundInterrupted : public DisplayLayout {
	DLButton _gBtn;
public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void tick() override;
};

#endif // DL_BGINTERRUPTED_H__