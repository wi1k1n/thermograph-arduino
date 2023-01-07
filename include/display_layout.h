#ifndef DISPLAY_LAYOUT_H__
#define DISPLAY_LAYOUT_H__

#include "sdk.h"
#include "display.h"
#include "sensor.h"
#include "interact.h"
#include "utilities.h"

class DLGUI {
protected:
    Display* _display = nullptr;
    bool _visible = false;
    bool _focused = false;
    
	inline Display& display() { return *_display; }
public:
    DLGUI() = default;
    virtual bool init(Display* display, bool visible = true, bool focused = false);
    virtual void tick() { }
    virtual void draw(bool doDisplay = true, bool clearFirst = true) { }
	virtual void clear(bool doDisplay = false) { }
    
    inline bool isVisible() const { return _visible; }
    inline bool isFocused() const { return _focused; }

    inline void setVisible(bool visible = true, bool reDraw = false, bool doDisplay = false);
    inline void setFocused(bool focused = true, bool reDraw = false, bool doDisplay = false);
};

class DLButton : public DLGUI {
    UPoint _pos;
    UPoint _size;
    String _title;

    uint8_t _textSize = 1;
    uint8_t _borderThickness = 1;
    UPoint _titleOffset;
public:
    bool init(Display* display, UPoint pos, UPoint size, const String& title, bool visible = true, bool focused = false);
    void tick() override { }
    void draw(bool doDisplay = false, bool clearFirst = true) override;
	void clear(bool doDisplay = false) override;
};

////////////////////////////////////

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

class DLayoutWelcome : public DisplayLayout {
	Timer _timer;
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void draw(bool doDisplay = true) override;
	void activate() override;
	void deactivate() override;
	void update(void* data) override;
	void tick() override;
};

class DLayoutBackgroundInterrupted : public DisplayLayout {
    DLButton _gBtn;
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void draw(bool doDisplay = true) override;
	void tick() override;
};

// Menu layouts
class DLayoutMain : public DisplayLayout {
	float _temp1;
    
    DLButton _gBtnStart;
    DLButton _gBtnResume;
    DLButton _gBtnStop;

	LED _debugLED;
	
	void drawGButtons(bool doDisplay = false);
	void adjustGButtonsModeInteract();
	void adjustGButtonsModeBGInterrupted();
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void activate() override;
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
};

class DLayoutGraph : public DisplayLayout {
	float _temp1;
public:
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
};

class DLayoutSettings : public DisplayLayout {
	float _temp1;
	Timer _timerRandomPixel;
public:
	bool init(Display* display, Application* app, PushButton* btn1, PushButton* btn2) override;
	void draw(bool doDisplay = true) override;
	void activate() override;
	void deactivate() override;
	void update(void* data) override;
	void tick() override;
};

#endif // DISPLAY_LAYOUT_H__