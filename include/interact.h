#ifndef INTERACT_H__
#define INTERACT_H__

#include "VirtualButton.h"

#define PUSHBUTTON_PIN_UNASSIGNED 0xff

class PushButton : public VButton {
    uint8_t _pin = PUSHBUTTON_PIN_UNASSIGNED;
public:
    enum STATE {
        NONE = 0,

    };
    PushButton() = default;
    PushButton(uint8_t pin) {
        init(pin);
    }
    bool init(uint8_t pin) {
        _pin = pin;
        pinMode(_pin, INPUT_PULLUP);
        return true;
    }
    bool tick() {
        return poll(!digitalRead(_pin));
    }
};

#endif // INTERACT_H__