#include "display.h"

template <class DisplayImpl>
bool Display<DisplayImpl>::setup() {
    SetupBase::setup();
    return _display.setup();
}