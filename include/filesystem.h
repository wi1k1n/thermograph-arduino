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

	template <typename T> static bool readStruct(File& f, T& s);
	template <typename T> static bool readStruct(const char* path, T& s);
	// template <typename T> static bool readStructVersioned(File& f, T& s, uint8_t& vMaj, uint8_t& vMin);
	// template <typename T> static bool readStructVersioned(const char* path, T& s, uint8_t& vMaj, uint8_t& vMin);

	template <typename T> static bool writeStruct(File& f, T& s);
	template <typename T> static bool writeStruct(const char* path, T& s);
};

//////////////////////////////////////////////

/// @brief Baseclass for any structures that are stored in filesystem
struct StorageStruct {
	StorageStruct() { }
	StorageStruct(const StorageStruct& other) { copyFrom(other); }
	virtual void copyFrom(const StorageStruct& other) { }
	virtual bool convertFromVersion(uint8_t vMaj, uint8_t vMin) { return false; }
};

/// @brief SStruct that contains info to be read after rebot in background task mode
struct SStrSleeping : StorageStruct {
	size_t timeAwake = 0;
	Application::Mode mode = Application::Mode::INTERACT;

	SStrSleeping() = default;
	SStrSleeping(const SStrSleeping& other) { copyFrom(other); }
	virtual void copyFrom(const SStrSleeping& other);
	virtual bool convertFromVersion(uint8_t vMaj, uint8_t vMin);

	bool isValid() const { return timeAwake > 0; }
};

/// @brief SStruct that keeps configuration parameters for the device itself
struct SStrConfig : StorageStruct {
	uint16_t periodCapture = 0;
	uint16_t nMeasurements = 0;
	uint16_t periodLive = 0;
	
	SStrConfig() = default;
	SStrConfig(const SStrConfig& other) { copyFrom(other); }
	virtual void copyFrom(const SStrConfig& other);
	virtual bool convertFromVersion(uint8_t vMaj, uint8_t vMin);

	bool isValid() const { return periodCapture && nMeasurements && periodLive; }
};

/// @brief SStruct that keeps measured data
struct SStrDatafile : StorageStruct {
	std::vector<uint8_t> data;
	
	SStrDatafile() = default;
	SStrDatafile(const SStrDatafile& other) { copyFrom(other); }
	virtual void copyFrom(const SStrDatafile& other);
	virtual bool convertFromVersion(uint8_t vMaj, uint8_t vMin);
};

//////////////////////////////////////////////

/// @brief A useful class that consolidates different SStructs and provides and interface to work with them
class Storage {
public:
	static bool init();
	
	static bool readSleeping();
	static bool writeSleeping();
	static bool removeSleeping();
	static const SStrSleeping& getSleeping() { return _sleeping; }
	static void setSleeping(size_t timeAwake, Application::Mode mode) {
		_sleeping.timeAwake = timeAwake;
		_sleeping.mode = mode;
	}


	static bool readConfig();
	static bool writeConfig();
	static bool removeConfig();
	static SStrConfig& getConfig() { return _config; }

	// static SStrConfig& getConfig(bool retrieve = false);
	// static bool storeConfig();

	static SStrDatafile& getDatafile(bool retrieve = false);
	static bool cleanDatafile();
	static bool addMeasurementData(uint8_t val);
private:
	
	static bool retreiveConfig(bool createNew = false);
	static bool retreiveDatafile(bool createNew = false);

private:
	static SStrConfig _config;
	static SStrSleeping _sleeping;
	static SStrDatafile _datafile;
};

#endif // FILESYSTEM_H__