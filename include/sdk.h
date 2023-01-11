#ifndef SDK_H__
#define SDK_H__

#include <Arduino.h>
#include <unordered_map>

// #define CUSTOM_UART
#define TDEBUG
#define SSD1306_NO_SPLASH

static const uint16_t MODE_DETECTION_DELAY = 250; // ms

static const int8_t DISPLAY_PIN_RESET = -1;

static const uint16_t DISPLAY_LAYOUT_LOGO_DELAY = 1000; // ms
// static const uint8_t DISPLAY_LAYOUT_PADDING_TOP = 16;
static const uint16_t DISPLAY_LAYOUT_MAIN_MEASUREMENT_PERIOD = 1000; // ms

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

#ifdef CUSTOM_UART
#include "CustomUART.h"
#define _UART_ uart
#else
#define _UART_ Serial
#endif

#ifdef TDEBUG
#define __PRIVATE_LOG_PREAMBULE	   (_UART_.print(millis())+\
									_UART_.print(" | ")+\
									_UART_.print(__FILE__)+\
									_UART_.print(F(":"))+\
									_UART_.print(__LINE__)+\
									_UART_.print(F(":"))+\
									_UART_.print(__func__)+\
									_UART_.print(F("() - ")))
#define DLOGLN(txt)		(__PRIVATE_LOG_PREAMBULE+_UART_.println(txt))
#define DLOGF(fmt, ...)	(__PRIVATE_LOG_PREAMBULE+_UART_.printf(fmt, __VA_ARGS__))
#define DLOG(txt)    	(__PRIVATE_LOG_PREAMBULE+_UART_.print(txt))
#define LOGLN(txt)		(_UART_.println(txt))
#define LOGF(fmt, ...)	(_UART_.printf(fmt, __VA_ARGS__))
#define LOG(txt)    	(_UART_.print(txt))
#else
#define DLOGLN(txt) do {} while(false)
#define DLOG(txt) do {} while(false)
#define LOGLN(txt) do {} while(false)
#define LOG(txt) do {} while(false)
#endif

#endif // SDK_H__