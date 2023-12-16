#ifndef DISPLAY_LAYOUT_H__
#define DISPLAY_LAYOUT_H__

#include <Arduino.h>

#include "../hardware/hwdisplay_interface.h"

class DisplayLayoutInterface {
protected:
	HWDisplayInterface* _display = nullptr;
	// Display* _display = nullptr;
	// Application* _app = nullptr;
	// HardwareInputs* _inputs = nullptr;
	// inline Display& display() { return *_display; }
public:
	DisplayLayoutInterface() = default;
	// virtual bool init(Display* display, Application* app, HardwareInputs* inputs);
	virtual bool init(HWDisplayInterface* hwDisplay);
	virtual void tick() { }
	virtual void draw() { }
	virtual void update(void* data) { } 				// Is called when layout needs to update its data (e.g. measurement value)
	
	virtual void transitionEnterStarted() { } 			// Is called at the very beginning of the transition onto this layout
	virtual void transitionEnterFinished() { draw(); } 	// Is called after the transition onto this layout finished
	virtual void transitionLeaveStarted() { } 			// Is called at the very beginning of the transition from this layout
	virtual void transitionLeaveFinished() { } 			// Is called after the transition from this layout finished
};

#endif // DISPLAY_LAYOUT_H__