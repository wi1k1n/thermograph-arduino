#include "display/display_layout.h"

bool DLGUI::init(Display* display, bool visible) {
    if (!display)
        return false;
    _display = display;
    setVisible(visible);
    return true;
}

void DLGUI::setVisible(bool visible, bool reDraw, bool doDisplay) {
	_visible = visible;
	if (reDraw) {
		if (_visible) {
			draw(doDisplay);
		} else {
			clear(doDisplay);
		}
	}
}

///////////////////////////////

bool DLButton::init(Display* display, UPoint pos, UPoint size, const String& title, bool visible, bool focused, bool pressed) {
    if (!DLGUI::init(display, visible) || !DLGUIFocusable::init(focused) || !DLGUIPressable::init(pressed))
        return false;
    _pos = pos;
    _size = size;

    if (size.y >= DISPLAY_FONT_HEIGHT * 2 && size.x >= title.length() * DISPLAY_FONT_WIDTH * 2) {
        _textSize = 2;
        _borderThickness = 2;
    }
    
	setTitle(title, false, false);

    return true;
}

void DLButton::draw(bool doDisplay, bool clearFirst) {
	// DLOG("DLButton.draw() - ");
	// LOGLN(_title);

	DLGUI::draw(doDisplay, clearFirst);
    if (!isVisible()) {
        return;
    }

	const uint8_t clrBG = isPressed() ? DISPLAY_WHITE : DISPLAY_BLACK;
	const uint8_t clrFG = isPressed() ? DISPLAY_BLACK : DISPLAY_WHITE;

	if (clearFirst && !isPressed()) {
		clear(false);
	}
    if (isPressed()) {
	    display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, clrBG);
	} else {
    	display()->drawRect(_pos.x, _pos.y, _size.x, _size.y, clrFG);
		if (isFocused()) {
			display()->drawRect(_pos.x + 1, _pos.y + 1, _size.x - 2, _size.y - 2, clrFG);
		}
	}
    
	display()->setTextColor(clrFG);
    display()->setCursor(_pos.x + _titleOffset.x, _pos.y + _titleOffset.y);
	display()->setTextSize(_textSize);
	display()->print(_title);
    
	if (doDisplay) {
		display()->display();
	}
}

void DLButton::clear(bool doDisplay) {
	display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, DISPLAY_BLACK);
	if (doDisplay) {
		display()->display();
	}
}

void DLButton::setTitle(const String& title, bool reDraw, bool doDisplay) {
    _title = title;
    const uint16_t titleWidth = title.length() * _textSize * DISPLAY_FONT_WIDTH - _textSize;
    const uint8_t titleHeight = _textSize * DISPLAY_FONT_HEIGHT - _textSize;
    _titleOffset.x = max((_size.x - titleWidth) / 2, 0);
    _titleOffset.y = max((_size.y - titleHeight) / 2, 0);

	if (reDraw) {
		draw(doDisplay);
	}
}

void DLButton::setFocused(bool focused, bool reDraw, bool doDisplay) {
	DLGUIFocusable::setFocused(focused);
	if (reDraw) {
		draw(doDisplay);
	}
}

void DLButton::setPressed(bool pressed, bool reDraw, bool doDisplay) {
	DLGUIPressable::setPressed(pressed);
	if (reDraw) {
		draw(doDisplay);
	}
}

/////////////////////////////////

bool DLSpinButton::init(Display* display, UPoint pos, UPoint size, const String& title, bool visible, bool focused, bool pressed, bool arrowLeft, bool arrowRight) {
    if (!DLButton::init(display, pos, size, title, visible, focused, pressed))
        return false;
	setArrows(arrowLeft, arrowRight);
    return true;
}

void DLSpinButton::setArrows(bool left, bool right) {
	_arrowLeft = left;
	_arrowRight = right;
}

void DLSpinButton::draw(bool doDisplay, bool clearFirst) {
	// DLOG("DLSpinButton.draw() - ");
	// LOGLN(_title);
	
	DLButton::draw(false, clearFirst);
    if (!isVisible()) {
        return;
    }

	const uint8_t clrBG = isPressed() ? DISPLAY_WHITE : DISPLAY_BLACK;
	const uint8_t clrFG = isPressed() ? DISPLAY_BLACK : DISPLAY_WHITE;

	const uint8_t padding = 3;
	const uint8_t arrowHeight = _size.y - padding * 2;
	const uint8_t arrowWidth = arrowHeight * .75;

	const uint8_t arrowY1 = _pos.y + padding; // upper corner
	const uint8_t arrowY2 = _pos.y + _size.y / 2; // mid corner
	const uint8_t arrowY3 = _pos.y + padding + arrowHeight; // bottom corner
	if (_arrowLeft) {
		const UPoint p1{ _pos.x + padding, arrowY2 };
		const UPoint p2{ _pos.x + padding + arrowWidth, arrowY1 };
		const UPoint p3{ _pos.x + padding + arrowWidth, arrowY3 };
		display()->fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, clrFG);
	}
	if (_arrowRight) {
		const UPoint p1{ _pos.x + _size.x - padding, arrowY2 };
		const UPoint p2{ _pos.x + _size.x - padding - arrowWidth, arrowY1 };
		const UPoint p3{ _pos.x + _size.x - padding - arrowWidth, arrowY3 };
		display()->fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, clrFG);
	}
    
	if (doDisplay) {
		display()->display();
	}
}

/////////////////////////////////

bool DisplayLayout::init(Display* display, Application* app, HardwareInputs* inputs) {
	if (!display || !app || !inputs)
		return false;
	_display = display;
	_app = app;
	_inputs = inputs;
	return true;
}