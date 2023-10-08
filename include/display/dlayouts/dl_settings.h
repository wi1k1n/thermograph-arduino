#ifndef DL_SETTINGS_H__
#define DL_SETTINGS_H__

#include "display/display_layout.h"

class Application;

class DLayoutSettings : public DisplayLayout {
protected:
	enum class SettingsMode {
		MENU = 0,
		OPTION_SELECTION,
		OPTION_CHANGING
	};
	enum class Options {
		PERIOD = 0,
		N_MEASUREMENTS,

		_OPTIONS_COUNT
	};
	enum class OptionChangeAction {
		NONE = 0,
		INCREMENT_SMALL,
		INCREMENT_STEP,
		DECREMENT_SMALL,
		DECREMENT_STEP
	};

public:
	bool init(Display* display, Application* app, HardwareInputs* inputs) override;
	void draw(bool doDisplay = true) override;
	void activate() override;
	void deactivate() override;
	void update(void* data) override;
	void tick() override;
protected:
	DLSpinButton& getOption(const Options& option);
	void changeMode(const SettingsMode& mode);
	void selectOption(uint8_t idx);

private:
	float _temp1;
	Timer _timerRandomPixel;

	SettingsMode _mode = SettingsMode::MENU;
	uint8_t _selectedOptionIdx = 0;

	std::vector<DLSpinButton> _options;

	uint8_t _periodIdx = 9;
};

#endif // DL_SETTINGS_H__