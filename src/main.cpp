#include "sdk.h"
#include "config.h"
#include "interaction.h"
#include "storage.h"
#include "display.h"

class Application : public SetupBase {
  ConfigurationManager    _config;
  StorageManager          _storage;
  DisplayInterface        _display = DisplaySSD1306();

  HardwareInteraction     _interactionHW;
public:
  bool setup() override;
  void loop();
};

bool Application::setup() {
  SetupBase::setup();

  bool setupSuccess = true;
  setupSuccess &= _storage.setup();
  setupSuccess &= _config.setup(&_storage);
  setupSuccess &= _display.setup();
  setupSuccess &= _interactionHW.setup(&_display);

  return setupSuccess;
}

void Application::loop() {
  _interactionHW.loop();
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