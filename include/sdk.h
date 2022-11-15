#ifndef SDK_H__
#define SDK_H__

#include <Arduino.h>
#include <unordered_map>

static const uint16_t DISPLAY_SCREEN_WIDTH = 128;
static const uint16_t DISPLAY_SCREEN_HEIGHT = 64;
static const int8_t DISPLAY_PIN_RESET = -1;

static const char STORAGEKEY_CONFIG[] PROGMEM = "config";

static const uint8_t INTERACTION_HW_PIN_B1 = 0; // D3

#define TDEBUG

#ifdef TDEBUG
#define __PRIVATE_LOG_PREAMBULE(txt)   do {\
                                            Serial.print(__FILE__);\
                                            Serial.print(F(":"));\
                                            Serial.print(__LINE__);\
                                            Serial.print(F(":"));\
                                            Serial.print(__func__);\
                                            Serial.print(F("() - "));\
                                        } while(false)
#define DLOGLN(txt)  do {\
                        __PRIVATE_LOG_PREAMBULE(txt);\
                        Serial.println(txt);\
                    } while(false)
#define DLOG(txt)    do {\
                        __PRIVATE_LOG_PREAMBULE(txt);\
                        Serial.print(txt);\
                    } while(false)
#define LOGLN(txt)  do {\
                        Serial.println(txt);\
                    } while(false)
#define LOG(txt)    do {\
                        Serial.print(txt);\
                    } while(false)
#else
#define DLOGLN(txt) do {} while(false)
#define DLOG(txt) do {} while(false)
#define LOGLN(txt) do {} while(false)
#define LOG(txt) do {} while(false)
#endif

class SetupBase {
    bool _ready{ false };
public:
    virtual bool setup() {
        _ready = true;
        return true;
    }
    virtual bool isReady() {
        return _ready;
    }
};

// typedef bool Bool;
// typedef signed char Char;
// typedef unsigned char UChar;
// typedef short int Short;
// typedef unsigned short int UShort;
// typedef long int Int;
// typedef unsigned long int UInt;
// typedef long long int Long;
// typedef unsigned long long int ULong;
// typedef float Float;
// typedef double Double;

// using Byte = UChar;
// using Int32 = Int;
// using UInt32 = UInt;
// using Int64 = Long;
// using UInt64 = ULong;

// template<typename K, typename V>
// class Hashmap : public std::unordered_map<K, V> {};



#endif // SDK_H__