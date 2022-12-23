#include "sdk.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Application {
public:
  bool setup();
  void loop();
};

bool Application::setup() {
  return true;
}

void Application::loop() {

}

Application app;

void setup() {
#ifdef TDEBUG
  Serial.begin(115200);
  delay(1);
  Serial.println();
#endif
  bool setupSuccess = app.setup();
  LOG(F("setup() -> "));
  LOGLN(static_cast<bool>(setupSuccess));
}
void loop() {
  app.loop();
}