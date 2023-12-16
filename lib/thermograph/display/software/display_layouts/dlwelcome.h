#ifndef DLWELCOME_H__
#define DLWELCOME_H__

#include <Arduino.h>

#include "sdk.h"

#include "../display_layout.h"

class DLWelcome : public DisplayLayoutInterface {
	// Timer _timer;
public:
	void tick() override;
	void draw() override;
	void update(void* data) override;
	
	void transitionEnterFinished() override;
	void transitionLeaveStarted() override;
};

#endif // DLWELCOME_H__