#include "filesystem.h"

Config Storage::_config;

bool Storage::init() {
	if (!LittleFS.begin()) {
		return false;
	}
	// TODO: make open/close easier to work with
	File f = LittleFS.open("/config", "r");
	if (!f) {
		// _config
	}
	f.close();
	return true;
}

bool Storage::isSleeping() {
	return LittleFS.exists("/sleeping");
}