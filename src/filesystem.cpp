#include "filesystem.h"
#include "sdk.h"

Config Storage::_config;

bool Storage::init() {
	if (!LittleFS.begin()) {
		return false;
	}
	// TODO: make open/close easier to work with
	AutoFile f = openR("/config");
	if (!f) {
		DLOGLN("No File!");
	} else {
		// AutoFile
	}
	return true;
}

AutoFile Storage::open(const char* path, const char* mode) {
	return AutoFile(LittleFS.open(path, mode));
}
AutoFile Storage::open(const String& path, const char* mode) {
	return AutoFile(LittleFS.open(path, mode));
}
AutoFile Storage::openR(const char* path) {
	return open(path, "r");
}
AutoFile Storage::openW(const char* path) {
	return open(path, "w");
}

bool Storage::isSleeping() {
	return LittleFS.exists("/sleeping");
}