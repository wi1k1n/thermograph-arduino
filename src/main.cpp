#include "Arduino.h"
#include "sdk.h"
#include "display.h"
#include "sensor.h"
#include "interact.h"
#include "utilities.h"

class Application {
  Display _display;

  PushButton _btn1;

  DLayoutWelcome _dlWelcome;
  DLayoutMain _dlMain;

  TempSensor _sensorTemp;

  Timer _DEBUG_randomPixel;

  void measureTemperature();
public:
  bool setup();
  void loop();
};

void Application::measureTemperature() {
  if (_sensorTemp.measure()) {
    TempSensorData* dataPtr = static_cast<TempSensorData*>(_sensorTemp.waitForMeasurement());
    if (dataPtr) {
      LOG(F("Temperature: "));
      LOGLN(dataPtr->temp);
      _dlMain.update(dataPtr);
    } else {
      LOGLN(F("Couldn't get measurement even after 1s!"));
    }
  } else {
    LOGLN(F("Couldn't start measuring temperature!"));
  }
}

bool Application::setup() {
  if (!_sensorTemp.init())
    return false;

  if (!_display.init(DISPLAY_SCREEN_WIDTH, DISPLAY_SCREEN_HEIGHT, &Wire, 0, 0x3C))
    return false;
  
  if (!_dlWelcome.init(&_display)) {
    return false;
  }
  if (!_dlMain.init(&_display)) {
    return false;
  }

  if (!_btn1.init(INTERACT_PUSHBUTTON_1_PIN)) {
    return false;
  }

  if (!_DEBUG_randomPixel.init(250, Timer::MODE::PERIOD)) {
    return false;
  }
  
  _display->clearDisplay();

  _dlWelcome.draw();
  delay(DISPLAY_LAYOUT_LOGO_DELAY);
  
  measureTemperature();
  _dlMain.draw();

  _DEBUG_randomPixel.start();

  return true;
}

void Application::loop() {
  _btn1.tick();

  if (_btn1.click()) {
    LOGLN(F("Button clicked!"));
  }

  if (_DEBUG_randomPixel.tick()) {
    int16_t x = random(DISPLAY_SCREEN_WIDTH),
            y = random(DISPLAY_SCREEN_HEIGHT);
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
  delay(5000);
}
void loop() {
  app.loop();
}