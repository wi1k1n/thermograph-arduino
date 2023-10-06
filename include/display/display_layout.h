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
    
    void setFocused(bool focused = true, bool reDraw = false, bool doDisplay = false);
    void setPressed(bool pressed = true, bool reDraw = false, bool doDisplay = false);
};

////////////////////////////////////

/// @brief Base class for any Display Layout class implementations.
class DisplayLayout {
protected:
	Display* _display = nullptr;
	Application* _app = nullptr;
	PushButton* _btn1 = nullptr;
	PushButton* _btn2 = nullptr;
	inline Display& display() { return *_display; }
public:
	DisplayLayout() = default;
	virtual bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2);
	virtual void activate() { draw(); } // should be called when layout is entered
	virtual void deactivate() { }
	virtual void draw(bool doDisplay = true) { }
	virtual void update(void* data) { }
	virtual void tick() { }
};

#endif // DISPLAY_LAYOUT_H__