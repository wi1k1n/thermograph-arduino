#include "Arduino.h"
#include "sdk.h"
#include "display.h"
#include "sensor.h"
#include "interact.h"
#include "utilities.h"
#include "TimerLED.h"

class Application {
  Display _display;
  TimerLED _displayErrorTimerLED;
  DLayoutWelcome _dlWelcome;
  DLayoutMain _dlMain;
  DLTransition _dltransMain;
  PushButton _btn1;
  TempSensor _sensorTemp;

  Timer _DEBUG_randomPixel;

  void measureTemperature();
  void showDisplayError();
public:
  bool setup();
  void loop();
};

void Application::measureTemperature() {
  if (_sensorTemp.measure()) {
    TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement());
    if (dataPtr) {
      _dlMain.update(dataPtr);
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
  if (!_sensorTemp.init()) {
    DLOGLN(initFailed);
    return false;
  }

  if (!_display.init(&Wire, 0, 0x3C)) {
    _displayErrorTimerLED.init(LED_BUILTIN);
    _displayErrorTimerLED.setIntervals(4, (const uint16_t[]){ 50, 100, 150, 100 });
    _displayErrorTimerLED.restart();
    DLOGLN(initFailed);
    return false;
  }
  
  if (!_dlWelcome.init(&_display)) {
    DLOGLN(initFailed);
    return false;
  }
  if (!_dlMain.init(&_display)) {
    DLOGLN(initFailed);
    return false;
  }

  if (!_dltransMain.init(&_display)) {
    return false;
  }

  if (!_btn1.init(INTERACT_PUSHBUTTON_1_PIN)) {
    DLOGLN(initFailed);
    return false;
  }

  if (!_DEBUG_randomPixel.init(200, Timer::MODE::PERIOD)) {
    DLOGLN(initFailed);
    return false;
  }
  
  _display->clearDisplay();

  _dlWelcome.draw();
  // delay(DISPLAY_LAYOUT_LOGO_DELAY);
  
  measureTemperature();

  _DEBUG_randomPixel.start();

  return true;
}

uint8_t type = 0;
void Application::loop() {
  if (_displayErrorTimerLED.isActive()) {
    _displayErrorTimerLED.tick();
  }

  _dltransMain.tick();

  if (_btn1.tick()) {
    if (_btn1.click()) {
      LOGLN(F("Button clicked!"));
      if (type == 0) {
        _dltransMain.start(&_dlWelcome, &_dlMain, Display::ScrollDir::LEFT);
      } else {
        _dltransMain.start(&_dlMain, &_dlWelcome, Display::ScrollDir::RIGHT);
      }
      type = (type + 1) % 2;
    }
  }

  if (_DEBUG_randomPixel.tick()) {
    int16_t x = random(_display->width()),
            y = random(_display->height());
    _display->drawPixel(x, y, DISPLAY_WHITE);
    _display->display();
  }
}

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