#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "sdk.h"
#include "config.h"

class Application {
  ConfigurationManager _config;
public:
  Bool setup();
  void loop();
};

bool Application::setup() {
  Bool configSetup = _config.setup();
  return true;
}

void Application::loop() {

}

Application app;
void setup() {
  app.setup();
}
void loop() {
  app.loop();
}