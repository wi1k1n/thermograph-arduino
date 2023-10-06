#ifndef DL_MAIN_H__
#define DL_MAIN_H__

#include "display/display_layout.h"

/// @brief Main DL implementation. Shows real-time sensor values and allows to start BG task.
class DLayoutMain : public DisplayLayout {
	float _temp1;
    
    DLButton _gBtnStart;
    DLButton _gBtnResume;
    DLButton _gBtnStop;

	Timer _timerMeasure;

	LED _debugLED;
	
	void drawGButtons(bool doDisplay = false);
	void adjustGButtonsModeInteract();
	void adjustGButtonsModeBGInterrupted();
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void activate() override;
	void deactivate() override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
};

#endif // DL_MAIN_H__