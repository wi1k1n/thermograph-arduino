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
    std::shared_ptr<DisplayInterface>         _display;
    EncButton<EB_TICK, INTERACTION_HW_PIN_B1> _button1;

public:
    bool setup(DisplayInterface* display) {
        if (!InteractionInterface::setup()) {
            return false;
        }

        _display.reset(display);
        if (!_display || !_display->isReady()) {
            DLOGLN(F("Display is not initialized!"));
            return false;
        }

        return true;
    }

    void loop() override {
        InteractionInterface::loop();
        _button1.tick();
    }
};

class WebserverInteraction : public InteractionInterface {

};

#endif // INTERACTION_H__