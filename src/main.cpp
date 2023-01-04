#include "main.h"

void Application::measureTemperature() {
  if (_sensorTemp.measure()) {
    TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement());
    if (dataPtr) {
      _dLayouts[DisplayLayoutKeys::MAIN]->update(dataPtr);
      LOG(F("Temperature: "));
      LOGLN(dataPtr->temp);
    } else {
      LOGLN(F("Couldn't get measurement even after 1s!"));
    }
  } else {
    LOGLN(F("Couldn't start measuring temperature!"));
  }
}

bool Application::setup() {
  String initFailed = F("init() failed!");
  if (!_display.init(&Wire, 0, 0x3C)) {
    _displayErrorTimerLED.init(LED_BUILTIN);
    _displayErrorTimerLED.setIntervals(4, (const uint16_t[]){ 50, 100, 150, 100 });
    _displayErrorTimerLED.restart();
    DLOGLN(initFailed);
    return false;
  }
  _display->clearDisplay();

  if (!_sensorTemp.init()) {
    DLOGLN(initFailed);
    return false;
  }
  
  // Order should follow the order in DisplayLayouts
  _dLayouts.push_back(std::make_unique<DLayoutWelcome>());
  _dLayouts.push_back(std::make_unique<DLayoutMain>());
  _dLayouts.push_back(std::make_unique<DLayoutGraph>());
  _dLayouts.push_back(std::make_unique<DLayoutSettings>());
  for (auto& dlayout : _dLayouts) {
    if (!dlayout->init(&_display, this, &_btn1, &_btn2)) {
      DLOGLN(initFailed);
      return false;
    }
  }

  if (!_dltransMain.init(&_display, 200, DLTransition::Interpolation::APOW3)) {
    return false;
  }

  if (!_btn1.init(INTERACT_PUSHBUTTON_1_PIN)) {
    DLOGLN(initFailed);
    return false;
  }
  if (!_btn2.init(INTERACT_PUSHBUTTON_2_PIN)) {
    DLOGLN(initFailed);
    return false;
  }
  
  _dLayouts[DisplayLayoutKeys::WELCOME]->draw();
  delay(DISPLAY_LAYOUT_LOGO_DELAY);
  activateDisplayLayout(DisplayLayoutKeys::MAIN, DLTransitionStyle::NONE);
  
  measureTemperature();

  return true;
}

void Application::loop() {
  if (_displayErrorTimerLED.isActive()) {
    _displayErrorTimerLED.tick();
  }

  _dltransMain.tick();
  _btn1.tick();
  _btn2.tick();
  getActiveDisplayLayout()->tick();
}

void Application::activateDisplayLayout(DisplayLayoutKeys dLayoutKey, DLTransitionStyle style) {
  if (dLayoutKey == _dLayoutActiveKey) {
    return;
  }
  DisplayLayout* target = _dLayouts[dLayoutKey].get();

  Display::ScrollDir direction = Display::ScrollDir::LEFT;
  switch (style) {
    case DLTransitionStyle::AUTO: {
      int8_t keyDst = dLayoutKey - _dLayoutActiveKey;
      bool jump = abs(keyDst) > 1;
      direction = ((keyDst < 0 && jump) || (keyDst > 0 && !jump)) ? Display::ScrollDir::LEFT : Display::ScrollDir::RIGHT;
      break;
    }
    case DLTransitionStyle::RIGHT: {
      direction = Display::ScrollDir::RIGHT;
      break;
    }
  }
  if (style == DLTransitionStyle::NONE) {
    target->activate();
  } else {
    _dltransMain.start(getActiveDisplayLayout(), target, direction);
  }
  _dLayoutActiveKey = dLayoutKey;
}

/////////////////////////

Application app;
void setup() {
#ifdef TDEBUG
  Serial.begin(115200);
  delay(1);
  Serial.println();
#endif
  const bool setupSucceeded = app.setup();
  DLOGLN(setupSucceeded);
}
void loop() {
  app.loop();
}