#include "display_layout.h"

bool DisplayLayoutInterface::init(HWDisplayInterface* hwDisplay) {
	_display = hwDisplay;
	return _display;
}
