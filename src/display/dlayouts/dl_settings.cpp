#include "display/dlayouts/dl_settings.h"
#include "application.h"

DLSpinButton& DLayoutSettings::getOption(const Options& option) {
	return _options.at(static_cast<size_t>(option));
}

const String periodLabels[] = { "5s", "10s", "15s", "30s", "1m", "1m 30s", "2m", "3m", "4m", "5m", "10m", "15m", "20m", "30m", "1h", "1h 30m", "2h", "3h", "4h", "5h", "6h", "8h", "10h", "12h", "18h" };
const uint16_t periodIntervals[] = { 5, 10, 15, 30, 60, 90, 120, 180, 240, 300, 600, 900, 1200, 1800, 3600, 5400, 7200, 10800, 14400, 18000, 21600, 28800, 36000, 43200, 64800 };

static uint8_t getPeriodIdxByInterval(uint16_t interval) {
	for (uint8_t i = 0; i < std::size(periodIntervals); ++i)
		if (interval == periodIntervals[i])
			return i;
	return 255;
}

String DLayoutSettings::retrieveOptionTitle(const Options& option) {
	ThSettings& settings = _app->getSettings();
	if (option == Options::PERIOD) {
		uint16_t period = settings.getEntry<uint16_t>(ThSettings::Entries::PERIOD_CAPTURE);
		uint8_t idx = getPeriodIdxByInterval(period);
		if (idx == 255)
			return "Error!";
		return "Period: " + String(periodLabels[idx]);
	}
	if (option == Options::N_MEASUREMENTS) {
		uint16_t nMeas = settings.getEntry<uint16_t>(ThSettings::Entries::N_MEASUREMENTS);
		return nMeas == static_cast<uint16_t>(-1) ? "Unlimited" : String(nMeas);
	}
	return "Error!";
}

bool DLayoutSettings::init(Display* display, Application* app, HardwareInputs* inputs) {
	bool success = DisplayLayout::init(display, app, inputs);

	const uint8_t bWidth = 120;
	const uint8_t bHeight = 15;
	const uint8_t bPadding = 1;
	const UPoint bSize{ bWidth, bHeight };
	const uint8_t dWidth = _display->rawWidth();
	const uint16_t bY = 16;

	_options.resize(static_cast<size_t>(Options::_OPTIONS_COUNT));

	success &= getOption(Options::PERIOD).init(display, {(dWidth - bWidth) / 2, bY}, bSize, retrieveOptionTitle(Options::PERIOD), 1, 0, 0, 0, 0);
	success &= getOption(Options::N_MEASUREMENTS).init(display, {(dWidth - bWidth) / 2, bY + bHeight + bPadding}, bSize, retrieveOptionTitle(Options::N_MEASUREMENTS), 1, 0, 0, 0, 0);

	return success && _timerRandomPixel.init(120, Timer::MODE::PERIOD);
}

void DLayoutSettings::update(void* data) {
	DisplayLayout::update(data);
}

void DLayoutSettings::transitionEnterFinished() {
	DisplayLayout::transitionEnterFinished();
	_timerRandomPixel.start();
}

void DLayoutSettings::transitionLeaveStarted() {
	DisplayLayout::transitionLeaveStarted();
	_timerRandomPixel.stop();
}

void DLayoutSettings::changeMode(const SettingsMode& mode) {
	_mode = mode;

	int16_t selectedIdx = _selectedOptionIdx;
	if (_mode == SettingsMode::MENU)
		selectedIdx = -1;

	for (uint8_t i = 0; i < _options.size(); ++i) {
		_options.at(i).setFocused(i == selectedIdx);
		_options.at(i).setArrows(0, 0);
	}
	
	if (_mode == SettingsMode::OPTION_CHANGING) {
		if (selectedIdx >= 0 && selectedIdx < _options.size()) {
			_options.at(selectedIdx).setArrows(1, 1);
		}
	}

	draw();
}

void DLayoutSettings::selectOption(uint8_t idx) {
	_selectedOptionIdx = idx;
	for (uint8_t i = 0; i < _options.size(); ++i) {
		_options.at(i).setFocused(i == idx);
	}
	draw();
	DLOG("New selected Idx: ");
	DLOGLN(_selectedOptionIdx);
}

void DLayoutSettings::tick() {
	DisplayLayout::tick();
	if (_timerRandomPixel.tick()) {
		int16_t x = random(display()->width()),
				y = random(display()->height());
		uint16_t clr = display()->getPixel(x, y) ? DISPLAY_BLACK : DISPLAY_WHITE;
		display()->drawPixel(x, y, clr);
		display()->display();
	}
	
	PushButton* btn1 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON1));
	PushButton* btn2 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON2));
	if (!btn1 || !btn2)
		return;

	if (!btn1->tick() && !btn2->tick()) // sanity
		return;

	// We are in the main menu selection mode
	if (_mode == SettingsMode::MENU) {
		if (!btn2->down() && btn1->click()) // main menu left
			return _app->activateDisplayLayout(DisplayLayoutKeys::MEASVIEWER);
		if (!btn1->down() && btn2->click()) // main menu right
			return _app->activateDisplayLayout(DisplayLayoutKeys::MAIN);

		if (!btn2->down() && btn1->held()) { // enter option selection mode
			return changeMode(SettingsMode::OPTION_SELECTION);
		}
	}

	// We are in the option selection mode
	if (_mode == SettingsMode::OPTION_SELECTION) {
		if (btn1->down() && btn2->down()) { // exit to main menu
			return changeMode(SettingsMode::MENU);
		}

		if (!btn1->down() && btn2->click()) { // next option
			return selectOption((_selectedOptionIdx + 1) % _options.size());
		}
		if (!btn2->down() && btn1->click()) { // prev option
			return selectOption((_selectedOptionIdx ? _selectedOptionIdx : _options.size()) - 1);
		}
		
		if (!btn2->down() && btn1->held()) { // enter option changing mode
			return changeMode(SettingsMode::OPTION_CHANGING);
		}
	}

	// We are in the option changing mode
	if (_mode == SettingsMode::OPTION_CHANGING) {
		if (btn1->down() && btn2->down()) { // exit to option selection mode
			_app->getSettings().storeConfig();
			return changeMode(SettingsMode::OPTION_SELECTION);
		}

		OptionChangeAction action = OptionChangeAction::NONE;
		uint16_t stepCounter = 0;
		if (!btn1->down() && btn2->click())
			action = OptionChangeAction::INCREMENT_SMALL;
		else if (!btn2->down() && btn1->click())
			action = OptionChangeAction::DECREMENT_SMALL;
		else if (!btn1->down() && btn2->step()) {
			action = OptionChangeAction::INCREMENT_STEP;
			stepCounter = btn2->getStepCounter();
		} else if (!btn2->down() && btn1->step()) {
			action = OptionChangeAction::DECREMENT_STEP;
			stepCounter = btn1->getStepCounter();
		}

		int8_t dir = 0;
		if (action == OptionChangeAction::INCREMENT_SMALL
			|| action == OptionChangeAction::INCREMENT_STEP)
			dir = 1;
		else if (action == OptionChangeAction::DECREMENT_SMALL
				|| action == OptionChangeAction::DECREMENT_STEP)
			dir = -1;
		
		if (action != OptionChangeAction::NONE) {
			if (_selectedOptionIdx == static_cast<uint8_t>(Options::PERIOD)) {
				uint8_t periodEntriesSize = std::size(periodIntervals);
				uint16_t curInterval = _app->getSettings().getEntry<uint16_t>(ThSettings::Entries::PERIOD_CAPTURE);
				uint8_t curIdx = getPeriodIdxByInterval(curInterval);
				uint8_t newIdx = ((curIdx + dir < 0 ? (periodEntriesSize - 1) : (curIdx + dir))) % periodEntriesSize;
				
				_app->getSettings().setEntry(ThSettings::Entries::PERIOD_CAPTURE, periodIntervals[newIdx]);
				getOption(Options::PERIOD).setTitle(retrieveOptionTitle(Options::PERIOD));

				draw();
			}
			
			if (_selectedOptionIdx == static_cast<uint8_t>(Options::N_MEASUREMENTS)) {
				const uint8_t thresholds[] = { 0, 10, 20, 30, static_cast<uint8_t>(-1) };
				const uint16_t increments[] = { 1, 10, 100, 1000, 10000 };
				int16_t increment = dir;
				for (uint8_t i = 0; i < std::size(thresholds); ++i) {
					if (stepCounter <= thresholds[i]) {
						increment *= increments[i];
						break;
					}
				}
				
				const uint16_t N = _app->getSettings().getEntry<uint16_t>(ThSettings::Entries::N_MEASUREMENTS);
				const uint16_t MAX_N = 65000; // must be lower than static_cast<uint16_t>(-2)
				int32_t newN = (N == static_cast<uint16_t>(-1) ? -1 : N) + increment;
				if (newN <= 0)
					newN = (increment > 0) - (increment < 0); // sign of increment
				else if (newN < 1)
					newN = 1;
				else if (newN > MAX_N)
					newN = MAX_N;
				const uint16_t finalN = static_cast<uint16_t>(newN);

				_app->getSettings().setEntry(ThSettings::Entries::N_MEASUREMENTS, finalN);
				getOption(Options::N_MEASUREMENTS).setTitle(retrieveOptionTitle(Options::N_MEASUREMENTS));

				draw();
			}
		}
	}
}

void DLayoutSettings::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Settings Layout"));

	for (DLButton& option : _options) {
		option.draw();
	}

	if (doDisplay)
		display()->display();
}