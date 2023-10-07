#include "display/display_layout.h"
#include "main.h"

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
    _title = title;

    if (size.y >= DISPLAY_FONT_HEIGHT * 2 && size.x >= title.length() * DISPLAY_FONT_WIDTH * 2) {
        _textSize = 2;
        _borderThickness = 2;
    }
    
    uint16_t titleWidth = title.length() * _textSize * DISPLAY_FONT_WIDTH - _textSize;
    uint8_t titleHeight = _textSize * DISPLAY_FONT_HEIGHT - _textSize;
    _titleOffset.x = max((_size.x - titleWidth) / 2, 0);
    _titleOffset.y = max((_size.y - titleHeight) / 2, 0);

    return true;
}

void DLButton::draw(bool doDisplay, bool clearFirst) {
	DLGUI::draw(doDisplay, clearFirst);
    if (!isVisible()) {
        return;
    }

	// DLOGLN();
	uint8_t clrBG = isPressed() ? DISPLAY_WHITE : DISPLAY_BLACK;
	uint8_t clrFG = isPressed() ? DISPLAY_BLACK : DISPLAY_WHITE;

	if (clearFirst && !isPressed()) {
		clear(false);
	}
    if (isPressed()) {
	    display()->fillRect(_pos.x, _pos.y, _size.x, _size.y, clrBG);
	} else {
    	display()->drawRect(_pos.x, _pos.y, _size.x, _size.y, clrFG);
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

bool DisplayLayout::init(Display* display, Application* app, HardwareInputs* inputs) {
	if (!display || !app || !inputs)
		return false;
	_display = display;
	_app = app;
	_inputs = inputs;
	return true;
}