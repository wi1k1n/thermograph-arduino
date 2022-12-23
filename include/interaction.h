#ifndef INTERACTION_H__
#define INTERACTION_H__

#include "sdk.h"
#include "display.h"
#include <EncButton.h>

#include <memory>

enum class InteractionHWButtonType {
    // Highly followed by EncButton lib
    PRESS = 1,
    CLICK,
    RELEASE,
    HELD,
    STEP,
    N_CLICKS
};

// The interaction interface is a layer between the interaction implementation 
// (physical buttons with display or web-page sending commands) and the core
class InteractionInterface {
public:
    virtual bool setup() { return true; }
    virtual void loop() {}
};

class HardwareInteraction : public InteractionInterface {
    DisplayInterface _display = DisplaySSD1306();
    EncButton<EB_TICK, INTERACTION_HW_PIN_B1> _button1;

public:
    bool setup() {
        if (!InteractionInterface::setup()) {
            return false;
        }
        return _display.setup();
    }

    void loop() override {
        InteractionInterface::loop();
        _button1.tick();
    }
};

class WebserverInteraction : public InteractionInterface {

};

#endif // INTERACTION_H__