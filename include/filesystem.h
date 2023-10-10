#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "application.h"
#include "LittleFS.h"

/// @brief Thermograph File System: A set of helping wrapping funcs for the LittleFS class
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

//////////////////////////////////////////////

/// @brief Baseclass for any structures that are stored in filesystem
struct StorageStruct {
	uint8_t versionMajor;
	uint8_t versionMinor;

	StorageStruct() : versionMajor(THERMOGRAPH_VERSION_MAJOR), versionMinor(THERMOGRAPH_VERSION_MINOR) { }
	StorageStruct(const StorageStruct&) = default;
};
/// @brief SStruct that contains info to be read after rebot in background task mode
struct SStrSleeping : StorageStruct {
	size_t timeAwake = 0;
	Application::Mode mode = Application::Mode::INTERACT;

	SStrSleeping() = default;
	SStrSleeping(const SStrSleeping&) = default;
};
/// @brief SStruct that keeps configuration parameters for the device itself
struct SStrConfig : StorageStruct {
	uint16_t periodCapture = 0;
	uint16_t nMeasurements = 0;
	uint16_t periodLive = 0;
	
	SStrConfig() = default;
	SStrConfig(const SStrConfig&) = default;
};

//////////////////////////////////////////////

/// @brief A useful class that consolidates different SStructs and provides and interface to work with them
class Storage {
	static bool retreiveSleeping();
	static bool retreiveConfig(bool createNew = false);
public:
	static bool init();
	
	static const SStrSleeping& getSleeping(bool retrieve = false);
	static bool setSleeping(size_t timeAwake, Application::Mode mode);
	static bool removeSleeping();

	static SStrConfig& getConfig(bool retrieve = false);
	static bool storeConfig();
private:
	static SStrConfig _config;
	static SStrSleeping _sleeping;
};

#endif // FILESYSTEM_H__