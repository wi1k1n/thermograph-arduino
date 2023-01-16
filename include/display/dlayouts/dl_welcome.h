#ifndef DL_WELCOME_H__
#define DL_WELCOME_H__

#include "display/display_layout.h"
#include "main.h"

class DLayoutWelcome : public DisplayLayout {
	Timer _timer;
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void draw(bool doDisplay = true) override;
	void activate() override;
	void deactivate() override;
	void update(void* data) override;
	void tick() override;
};

#endif // DL_WELCOME_H__