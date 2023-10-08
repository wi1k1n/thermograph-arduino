#include "application.h"

Application app;
void setup() {
	const bool setupSucceeded = app.setup();
	DLOGLN(setupSucceeded);
}
void loop() {
	app.loop();
}


// // Sanity Test deep sleep
// void setup() {
//   _UART_.begin(115200);
//   while(!_UART_) { }
//   _UART_.println();
//   _UART_.println("Start device in normal mode!");
 
//   delay(5000);
//   // Wait for serial to initialize.
//   while(!_UART_) { }
 
//   // Deep sleep mode for 10 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
//   _UART_.println("I'm awake, but I'm going into deep sleep mode for 10 seconds");
//   ESP.deepSleep(10e6);
// }
 
// void loop() {
// }

// // Sanity test display
// Display _display;
// void setup() {
// 	_UART_.begin(115200);
// 	delay(1);
// 	_UART_.println();
// 	_display.init(&Wire, 0, 0x3C);
// 	delay(100);

// 	_display->clearDisplay();

// 	_display->setTextColor(DISPLAY_WHITE);
// 	_display->setCursor(0, 0);
// 	_display->setTextSize(1);
// 	_display->print(F("Thermograph v2"));

// 	_display->display();
// }
// void loop() {
// 	_UART_.print(millis());
// 	_UART_.println(" hey!");
// 	delay(500);
// }

// // LittleFS serial file explorer
// void setup() {
// 	_UART_.begin(115200);
// 	delay(1);
// 	_UART_.println();
// 	LittleFS.begin();
// }
// void loop() {
// 	DEBUG::_debug();
// }