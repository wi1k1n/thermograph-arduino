#include "filesystem.h"
#include "sdk.h"

bool ThFS::init() { return LittleFS.begin(); }
bool ThFS::exists(const char* path) { return LittleFS.exists(path); }
bool ThFS::exists(const String& path) { return LittleFS.exists(path); }
bool ThFS::remove(const char* path) { return LittleFS.remove(path); }
bool ThFS::remove(const String& path) { return LittleFS.remove(path); }
File ThFS::open(const char* path, const char* mode) { return LittleFS.open(path, mode); }
File ThFS::open(const String& path, const char* mode) { return File(LittleFS.open(path, mode)); }
File ThFS::openR(const char* path) { return open(path, "r"); }
File ThFS::openW(const char* path) { return open(path, "w"); }
File ThFS::openA(const char* path) { return open(path, "a"); }
File ThFS::openRW(const char* path) { return open(path, "r+"); }
File ThFS::openRA(const char* path) { return open(path, "a+"); }
template <typename T>
bool ThFS::readStruct(File& f, T& s) {
	int availableBytes = f.available();
	// DLOGLN(availableBytes);
	size_t byteCount = sizeof(T);
	// DLOGLN(byteCount);
	if (byteCount > availableBytes)
		return false;
	char buffer[byteCount];
	size_t byteCountRead = f.readBytes(buffer, byteCount);
	// DLOGLN(byteCountRead);
	if (byteCountRead != byteCount)
		return false;
	s = T(*reinterpret_cast<T*>(buffer));
	return true;
}
template <typename T>
bool ThFS::writeStruct(File& f, T& s) {
	size_t byteCount = sizeof(T);
	// DLOGLN(byteCount);
	size_t byteCountWritten = f.write(reinterpret_cast<char*>(&s), byteCount);
	// DLOGLN(byteCountWritten);
	return byteCountWritten == byteCount;
}

/// ////////////////////////////////////

uint8_t StorageStruct::versionMajor = THERMOGRAPH_VERSION_MAJOR;
uint8_t StorageStruct::versionMinor = THERMOGRAPH_VERSION_MINOR;
SStrConfig Storage::_config;
SStrSleeping Storage::_sleeping;

bool Storage::init() {
	if (!ThFS::init())
		return false;
	if (!retreiveConfig(true))
		return false;
	return true;
}

bool Storage::retreiveConfig(bool createNew) {
	bool exists = ThFS::exists(STORAGEKEY_CONFIG);
	if (exists) {
		// DLOGLN("exists");
		File f = ThFS::openR(STORAGEKEY_CONFIG);
		if (!f)
			return false;
		// DLOGLN("f");
		bool success = ThFS::readStruct(f, _config);
		// DLOGLN(success);
		return success;
	}
	// DLOGLN("doesn't exists");
	if (!createNew)
		return false;
	// DLOGLN("createNew");
	File f = ThFS::openW(STORAGEKEY_CONFIG);
	if (!f)
		return false;
	// DLOGLN("f");
	bool success = ThFS::writeStruct(f, _config);
	// DLOGLN(success);
	return success;
}

bool Storage::retreiveSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING))
		return false;
	File f = ThFS::openR(STORAGEKEY_ISSLEEPING);
	if (!f)
		return false;
	return ThFS::readStruct(f, _sleeping);
}

const SStrSleeping& Storage::getSleeping(bool retrieve) {
	if (retrieve) {
		retreiveSleeping();
	}
	return _sleeping;
}

bool Storage::setSleeping(size_t timeAwake, Application::Mode mode) {
	File f = ThFS::openW(STORAGEKEY_ISSLEEPING);
	if (!f)
		return false;
	_sleeping.timeAwake = timeAwake;
	_sleeping.mode = mode;
	return ThFS::writeStruct(f, _sleeping);
}

bool Storage::removeSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING))
		return false;
	return ThFS::remove(STORAGEKEY_ISSLEEPING);
}