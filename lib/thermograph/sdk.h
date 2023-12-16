#ifndef SDK_H__
#define SDK_H__

#include <Arduino.h>

#ifdef CUSTOM_UART
#include "CustomUART.h"
#define _UART_ uart
#else
#define _UART_ Serial
#endif

// ================================================================================================
// ========== Debugging toolset ===================================================================
// ================================================================================================

static void __DEBUG_PRINT_PREAMBULE() {
	_UART_.print("T:");
	_UART_.print(millis());
	_UART_.print(" | ");
	_UART_.print(__FILE__);
	_UART_.print(":");
	_UART_.print(__LINE__);
	_UART_.print(":");
	_UART_.print(__func__);
	_UART_.print("() | ");
}
static void LOGLN() {
	_UART_.println();
}
template<typename T>
static void LOG(T v) {
	_UART_.print(v);
}
template<typename T>
static void LOGLN(T v) {
	_UART_.println(v);
}
template<typename T>
static void DLOG(T v) {
	__DEBUG_PRINT_PREAMBULE();
	LOG(v);
}
template<typename T>
static void DLOGLN(T v) {
	DLOG(v);
	LOGLN();
}
template <typename T, typename... Ts>
static void LOG(T v, Ts... ts) {
	LOG(v);
	LOG(ts...);
}
template <typename T, typename... Ts>
static void DLOG(T v, Ts... ts) {
	DLOG(v);
	LOG(ts...);
}
template <typename T, typename... Ts>
static void LOGLN(T v, Ts... ts) {
	LOG(v);
	LOG(ts...);
	LOGLN();
}
template <typename T, typename... Ts>
static void DLOGLN(T v, Ts... ts) {
	DLOG(v);
	LOG(ts...);
	LOGLN();
}

template <typename T>
static bool ASSERTPTR(const T* ptr) {
	if (!ptr)
		DLOGLN("Assert failed: nullptr!");
	return ptr;
}

#endif // SDK_H__