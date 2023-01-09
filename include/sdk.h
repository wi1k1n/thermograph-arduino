#ifndef SDK_H__
#define SDK_H__

#include <Arduino.h>
#include <unordered_map>

#define TDEBUG
#define SSD1306_NO_SPLASH

static const uint16_t MODE_DETECTION_DELAY = 250; // ms

static const int8_t DISPLAY_PIN_RESET = -1;

static const uint16_t DISPLAY_LAYOUT_LOGO_DELAY = 1000; // ms
static const uint8_t DISPLAY_LAYOUT_PADDING_TOP = 16;

static const uint8_t INTERACT_PUSHBUTTON_1_PIN = 14; //D5 0; // D3
static const uint8_t INTERACT_PUSHBUTTON_2_PIN = 12; //D6 2; // D4

static const char STORAGEKEY_CONFIG[] PROGMEM = "config";

enum DisplayLayoutKeys {
	NONE = -1,
	WELCOME = 0,
	BACKGROUND_INTERRUPTED,
	// Menu layouts
	MAIN,
	GRAPH,
	SETTINGS,

	_COUNT
};
enum class DLTransitionStyle {
	NONE = 0,
	AUTO,
	LEFT,
	RIGHT
};

#ifdef TDEBUG
// #define __PRIVATE_LOG_PREAMBULE(txt)   do {\
// 											Serial.print(__FILE__);\
// 											Serial.print(F(":"));\
// 											Serial.print(__LINE__);\
// 											Serial.print(F(":"));\
// 											Serial.print(__func__);\
// 											Serial.print(F("() - "));\
// 										} while(false)
// #define DLOGLN(txt)  do {\
// 						__PRIVATE_LOG_PREAMBULE(txt);\
// 						Serial.println(txt);\
// 					} while(false)
// #define DLOG(txt)    do {\
// 						__PRIVATE_LOG_PREAMBULE(txt);\
// 						Serial.print(txt);\
// 					} while(false)
// #define LOGLN(txt)	do {\
// 						Serial.println(txt);\
// 					} while(false)
// #define LOG(txt)    do {\
// 						Serial.print(txt);\
// 					} while(false)
#define __PRIVATE_LOG_PREAMBULE	   (Serial.print(millis())+\
									Serial.print(" | ")+\
									Serial.print(__FILE__)+\
									Serial.print(F(":"))+\
									Serial.print(__LINE__)+\
									Serial.print(F(":"))+\
									Serial.print(__func__)+\
									Serial.print(F("() - ")))
#define DLOGLN(txt)		(__PRIVATE_LOG_PREAMBULE+Serial.println(txt))
#define DLOGF(fmt, ...)	(__PRIVATE_LOG_PREAMBULE+Serial.printf(fmt, __VA_ARGS__))
#define DLOG(txt)    	(__PRIVATE_LOG_PREAMBULE+Serial.print(txt))
#define LOGLN(txt)		(Serial.println(txt))
#define LOGF(fmt, ...)	(Serial.printf(fmt, __VA_ARGS__))
#define LOG(txt)    	(Serial.print(txt))
#else
#define DLOGLN(txt) do {} while(false)
#define DLOG(txt) do {} while(false)
#define LOGLN(txt) do {} while(false)
#define LOG(txt) do {} while(false)
#endif

#endif // SDK_H__