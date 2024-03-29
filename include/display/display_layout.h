#ifndef DISPLAY_LAYOUT_H__
#define DISPLAY_LAYOUT_H__

#include "sdk.h"
#include "display.h"
#include "sensor.h"
#include "utilities.h"

/// @brief Base class for any Display Layout GUI element
class DLGUI {
protected:
	Display* _display = nullptr;
	bool _visible = false;
	
	inline Display& display() { return *_display; }
public:
	DLGUI() = default;
	virtual bool init(Display* display, bool visible = true);
	virtual void tick() { }
	virtual void draw(bool doDisplay = true, bool clearFirst = true) { }
	virtual void clear(bool doDisplay = false) { }
	
	inline bool isVisible() const { return _visible; }
	void setVisible(bool visible = true, bool reDraw = false, bool doDisplay = false);
};

/// @brief Base class for DLGUI elements that implement 'focusable' behavior
class DLGUIFocusable {
	bool _focused = false;
public:
	bool init(bool focused = false) { setFocused(focused); return true; }
	inline bool isFocused() const { return _focused; }
	void setFocused(bool focused = true) { _focused = focused; }
};

/// @brief Base class for DLGUI elements that implement 'pressable' behavior
class DLGUIPressable {
	bool _pressed = false;
public:
	bool init(bool pressed = false) { setPressed(pressed); return true; }
	inline bool isPressed() const { return _pressed; }
	void setPressed(bool pressed = true)  { _pressed = pressed; }
};

/// @brief DLGUI element that implements a button: contains title, can be focused and pressed
class DLButton : public DLGUI, public DLGUIFocusable, public DLGUIPressable {
protected:
	UPoint _pos;
	UPoint _size;
	String _title;

	uint8_t _textSize = 1;
	uint8_t _borderThickness = 1;
	UPoint _titleOffset;
public:
	bool init(Display* display, UPoint pos, UPoint size, const String& title, bool visible = true, bool focused = false, bool pressed = false);
	void tick() override { }
	void draw(bool doDisplay = false, bool clearFirst = true) override;
	void clear(bool doDisplay = false) override;
	
	void setTitle(const String& newTitle, bool reDraw = false, bool doDisplay = false);
	void setFocused(bool focused = true, bool reDraw = false, bool doDisplay = false);
	void setPressed(bool pressed = true, bool reDraw = false, bool doDisplay = false);
};

class DLSpinButton : public DLButton {
public:
	bool init(Display* display, UPoint pos, UPoint size, const String& title, bool visible = true, bool focused = false, bool pressed = false, bool arrowLeft = true, bool arrowRight = true);

	void draw(bool doDisplay = false, bool clearFirst = true) override;

	void setArrows(bool left, bool right);
private:
	bool _arrowLeft = true;
	bool _arrowRight = true;
};

////////////////////////////////////

class Application;

/// @brief Base class for any Display Layout class implementations.
class DisplayLayout {
protected:
	Display* _display = nullptr;
	Application* _app = nullptr;
	HardwareInputs* _inputs = nullptr;
	inline Display& display() { return *_display; }
public:
	DisplayLayout() = default;
	virtual bool init(Display* display, Application* app, HardwareInputs* inputs);
	virtual void draw(bool doDisplay = true) { } // doDisplay forces _display->display() in the end of the function
	virtual void update(void* data) { } 				// Is called when layout needs to update its data (e.g. measurement value)
	virtual void tick() { }
	
	virtual void transitionEnterStarted() { } 			// Is called at the very beginning of the transition to this layout
	virtual void transitionEnterFinished() { draw(); } 	// Is called after the transition to this layout finished
	virtual void transitionLeaveStarted() { } 			// Is called at the very beginning of the transition from this layout
	virtual void transitionLeaveFinished() { } 			// Is called after the transition from this layout finished
};

#endif // DISPLAY_LAYOUT_H__