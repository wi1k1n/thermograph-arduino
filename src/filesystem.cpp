#include "filesystem.h"

bool Storage::init() {
	return LittleFS.begin();
}