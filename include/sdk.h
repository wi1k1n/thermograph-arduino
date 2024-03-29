#ifndef SDK_H__
#define SDK_H__

#include <Arduino.h>
#include <unordered_map>

// #define CUSTOM_UART
#define TDEBUG
#define SSD1306_NO_SPLASH

#define THERMOGRAPH_VERSION_MAJOR 2
#define THERMOGRAPH_VERSION_MINOR 1

static const uint16_t MODE_DETECTION_DELAY = 150; // ms


static const uint16_t DISPLAY_LAYOUT_LOGO_DELAY = 800; // ms
// static const uint8_t DISPLAY_LAYOUT_PADDING_TOP = 16;
static const uint16_t DISPLAY_LAYOUT_MAIN_MEASUREMENT_PERIOD = 1000; // ms

static const uint8_t DISPLAY_PIN_RESET = 0;
static const uint8_t DISPLAY_ADDRESS = 0x3C;
static const uint8_t INTERACT_PUSHBUTTON_1_PIN = 14; //D5 0; // D3
static const uint8_t INTERACT_PUSHBUTTON_2_PIN = 12; //D6 2; // D4

static const uint8_t NTCTHERMISTOR_PIN = A0; // ADC pin to where NTC thermistor is connected
static const uint16_t NTCTHERMISTOR_R1_OHMS = 10000; // Resistance of the voltage divider resistor https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/

static const char STORAGEKEY_CONFIG[] 				= "/config"; 				// 
static const char STORAGEKEY_ISSLEEPING[] 			= "/issleeping"; 			//
static const char STORAGEKEY_DATAFILE[] 			= "/datafile"; 				//
static const char STORAGEKEY_DATAFILE_CONTAINER[] 	= "/datafile_container"; 	//

// The order in this enum must follow the order of inserting objects in Application class
// Main menu layouts should go consequently for DLTransitionStyle::AUTO to function properly
enum DisplayLayoutKeys {
	NONE = -1,
	WELCOME = 0,
	BACKGROUND_INTERRUPTED,
	GRAPH,
	// Menu layouts
	MAIN,
	MEASVIEWER,
	SETTINGS,
	//
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