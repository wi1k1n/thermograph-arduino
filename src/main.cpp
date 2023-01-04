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
    if (!dlayout->init(&_display, this)) {
      DLOGLN(initFailed);
      return false;
    }
  }

  if (!_dltransMain.init(&_display)) {
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

  if (!_DEBUG_randomPixel.init(120, Timer::MODE::PERIOD)) {
    DLOGLN(initFailed);
    return false;
  }
  
  _dLayouts[DisplayLayoutKeys::WELCOME]->draw();
  delay(DISPLAY_LAYOUT_LOGO_DELAY);
  activateDisplayLayout(DisplayLayoutKeys::MAIN);
  
  measureTemperature();
  _DEBUG_randomPixel.start();

  return true;
}

void Application::loop() {
  if (_displayErrorTimerLED.isActive()) {
    _displayErrorTimerLED.tick();
  }

  _dltransMain.tick();

  if (_btn1.tick() || _btn2.tick()) {
    if (_btn1.tick())
      DLOGLN(F("btn1 tick"));
    if (_btn2.tick())
      DLOGLN(F("btn2 tick"));
    getActiveDisplayLayout()->input(_btn1, _btn2);
  }

  if (_DEBUG_randomPixel.tick()) {
    int16_t x = random(_display->width()),
            y = random(_display->height());
    _display->drawPixel(x, y, DISPLAY_WHITE);
    _display->display();
  }
}

void Application::activateDisplayLayout(DisplayLayoutKeys dLayoutKey) {
  if (dLayoutKey == _dLayoutActiveKey) {
    return;
  }
  DisplayLayout* target = _dLayouts[dLayoutKey].get();
  int8_t keyDst = dLayoutKey - _dLayoutActiveKey;
  bool jump = abs(keyDst) > 1;
  Display::ScrollDir direction = ((keyDst < 0 && jump) || (keyDst > 0 && !jump)) ? Display::ScrollDir::LEFT : Display::ScrollDir::RIGHT;
  _dltransMain.start(getActiveDisplayLayout(), target, direction);
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