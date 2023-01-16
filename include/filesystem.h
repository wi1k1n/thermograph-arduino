#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "main.h"
#include "LittleFS.h"

class ThFS {
public:
	static bool init();

    static bool exists(const char* path);
    static bool exists(const String& path);
    static bool remove(const char* path);
    static bool remove(const String& path);
	
    static File open(const char* path, const char* mode);
    static File open(const String& path, const char* mode);
	static File openR(const char* path);
	static File openW(const char* path);
	static File openA(const char* path);
	static File openRW(const char* path);
	static File openRA(const char* path);

	template <typename T>
	static bool readStruct(File& f, T& s, bool ignoreVersion = false);
	template <typename T>
	static bool writeStruct(File& f, T& s);
};

/// //////////////////////////////

struct StorageStruct {
	uint8_t versionMajor;
	uint8_t versionMinor;

	StorageStruct() : versionMajor(THERMOGRAPH_VERSION_MAJOR), versionMinor(THERMOGRAPH_VERSION_MINOR) { }
	StorageStruct(const StorageStruct&) = default;
};
struct SStrSleeping : StorageStruct {
	size_t timeAwake = 0;
	Application::Mode mode = Application::Mode::INTERACT;

	SStrSleeping() = default;
	SStrSleeping(const SStrSleeping&) = default;
};
struct SStrConfig : StorageStruct {
	SStrConfig() = default;
	SStrConfig(const SStrConfig&) = default;
};

class Storage {
	static bool retreiveSleeping();
	static bool retreiveConfig(bool createNew = false);
public:
	static bool init();
	
	static const SStrSleeping& getSleeping(bool retrieve = false);
	static bool setSleeping(size_t timeAwake, Application::Mode mode);
	static bool removeSleeping();
private:
	static SStrConfig _config;
	static SStrSleeping _sleeping;
};

#endif // FILESYSTEM_H__