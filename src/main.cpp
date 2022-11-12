#include "sdk.h"
#include "config.h"
#include "interaction.h"
#include "storage.h"

class Application : public SetupBase {
  ConfigurationManager    _config;
  StorageManager          _storage;
  HardwareInteraction     _interactHW;
  WebserverInteraction    _interactWS;
public:
  bool setup() override;
  void loop();
};

bool Application::setup() {
  SetupBase::setup();

  bool setupSuccess = true;
  setupSuccess &= _storage.setup();
  setupSuccess &= _config.setup(&_storage);
  return setupSuccess;
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