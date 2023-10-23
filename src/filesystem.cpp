#include "filesystem.h"
#include "sdk.h"
#include <type_traits>

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
bool ThFS::readStruct(File& f, T& s, bool ignoreVersion) {
	static_assert(std::is_base_of_v<StorageStruct, T>);
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
	s.copyFrom(T(*reinterpret_cast<T*>(buffer)));
	if (ignoreVersion)
		return true;
	StorageStruct* ss = static_cast<StorageStruct*>(&s);
#ifdef TDEBUG
	if (!ss) {
		DLOGLN(F("!ss"));
	} else if (ss->versionMajor != THERMOGRAPH_VERSION_MAJOR || ss->versionMinor != THERMOGRAPH_VERSION_MINOR) {
		DLOG(F("File ")); LOG(f.fullName()); LOG(F(" has wrong version: "));
		LOG(ss->versionMajor); LOG(F(".")); LOG(ss->versionMinor);
		LOG(F(". Expected: ")); LOG(THERMOGRAPH_VERSION_MAJOR); LOG(F(".")); LOGLN(THERMOGRAPH_VERSION_MINOR);
	}
#endif
	return ss && ss->versionMajor == THERMOGRAPH_VERSION_MAJOR && ss->versionMinor == THERMOGRAPH_VERSION_MINOR;
}
template <typename T>
bool ThFS::writeStruct(File& f, T& s) {
	static_assert(std::is_base_of_v<StorageStruct, T>);
	size_t byteCount = sizeof(T);
	// DLOGLN(byteCount);
	size_t byteCountWritten = f.write(reinterpret_cast<char*>(&s), byteCount);
	// DLOGLN(byteCountWritten);
	return byteCountWritten == byteCount;
}

/// ////////////////////////////////////

// TODO: what a disgusting coding pattern here :-(
// TODO: cow pattern?
void StorageStruct::copyFrom(const StorageStruct& other) {
	versionMajor = other.versionMajor;
	versionMinor = other.versionMinor;
}
void SStrSleeping::copyFrom(const SStrSleeping& other) {
	StorageStruct::copyFrom(other);
	timeAwake = other.timeAwake;
	mode = other.mode;
}
void SStrConfig::copyFrom(const SStrConfig& other) {
	StorageStruct::copyFrom(other);
	periodCapture = other.periodCapture;
	nMeasurements = other.nMeasurements;
	periodLive = other.periodLive;
}
void SStrDatafile::copyFrom(const SStrDatafile& other) {
	StorageStruct::copyFrom(other);
	data = other.data;
}

bool StorageStruct::readFromFile(File& f) {
	if (!f)
		return false;
	return ThFS::readStruct(f, *this);
}
bool StorageStruct::writeToFile(File& f) {
	if (!f)
		return false;
	return ThFS::writeStruct(f, *this);
}
bool SStrSleeping::readFromFile(File& f) {
	if (!f)
		return false;
	return ThFS::readStruct(f, *this);
}
bool SStrSleeping::writeToFile(File& f) {
	if (!f)
		return false;
	return ThFS::writeStruct(f, *this);
}
bool SStrConfig::readFromFile(File& f) {
	if (!f)
		return false;
	return ThFS::readStruct(f, *this);
}
bool SStrConfig::writeToFile(File& f) {
	if (!f)
		return false;
	return ThFS::writeStruct(f, *this);
}


bool SStrDatafile::readFromFile(File& f) {
	if (!f)
		return false;
	DLOG("Reading datagile: [");
	StorageStruct::readFromFile(f);
	
	char buffer[12];
	if (f.readBytes(buffer, sizeof(uint16_t)) != sizeof(uint16_t)) {
		DLOGLN("Error reading data length!");
		return false;
	}
	
	uint16_t dSize;
	memcpy(&dSize, buffer, sizeof(uint16_t));
	LOG(dSize);
	LOG("]");

	if (!dSize)
		return true;
	
	data.resize(dSize);
	for (uint16_t i = 0; i < dSize; ++i) {
		if (f.readBytes(buffer, sizeof(uint8_t)) != sizeof(uint8_t)) {
			DLOGLN("Error reading data!");
			return false;
		}
		memcpy(&data[i], buffer, sizeof(uint8_t));
		LOG(" ");
		LOG(data[i]);
	}

	return true;
}
bool SStrDatafile::writeToFile(File& f) {
	if (!f)
		return false;
	StorageStruct::writeToFile(f);

	f.write(static_cast<uint16_t>(data.size()));
	for (auto v : data) {
		f.write(v);
	}
	return true;
}

// uint8_t StorageStruct::versionMajor = THERMOGRAPH_VERSION_MAJOR;
// uint8_t StorageStruct::versionMinor = THERMOGRAPH_VERSION_MINOR;
SStrConfig Storage::_config;
SStrSleeping Storage::_sleeping;
SStrDatafile Storage::_datafile;

/// ////////////////////////////////////

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
		bool success = _config.readFromFile(f);
		DLOG("Config retrieved! Values: ");
		LOG(_config.periodCapture);
		LOG(" | ");
		LOG(_config.nMeasurements);
		LOG(" | ");
		LOGLN(_config.periodLive);
		return success;
		// bool success = ThFS::readStruct(f, _config);
		// // DLOGLN(success);
		// return success;
	}
	// DLOGLN("doesn't exists");
	if (!createNew)
		return false;
	// DLOGLN("createNew");
	File f = ThFS::openW(STORAGEKEY_CONFIG);
	if (!f)
		return false;
	// DLOGLN("f");
	return _config.writeToFile(f);
	// bool success = ThFS::writeStruct(f, _config);
	// // DLOGLN(success);
	// return success;
}

bool Storage::retreiveSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING))
		return false;
	File f = ThFS::openR(STORAGEKEY_ISSLEEPING);
	if (!f)
		return false;
	return _sleeping.readFromFile(f);
	// return ThFS::readStruct(f, _sleeping);
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
	return _sleeping.writeToFile(f);
	// return ThFS::writeStruct(f, _sleeping);
}

bool Storage::removeSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING))
		return false;
	return ThFS::remove(STORAGEKEY_ISSLEEPING);
}

SStrConfig& Storage::getConfig(bool retrieve) {
	if (retrieve) {
		retreiveConfig();
	}
	return _config;
}

bool Storage::storeConfig() {
	File f = ThFS::openW(STORAGEKEY_CONFIG);
	if (!f)
		return false;
	return _config.writeToFile(f);
	// return ThFS::writeStruct(f, _config);
}

bool Storage::retreiveDatafile(bool createNew) {
	bool exists = ThFS::exists(STORAGEKEY_DATAFILE);
	if (exists) {
		File f = ThFS::openR(STORAGEKEY_DATAFILE);
		if (!f)
			return false;
		return _datafile.readFromFile(f);
	}
	if (!createNew)
		return false;
	File f = ThFS::openW(STORAGEKEY_CONFIG);
	if (!f)
		return false;
	return _datafile.writeToFile(f);
}

SStrDatafile& Storage::getDatafile(bool retrieve) {
	if (retrieve) {
		retreiveDatafile();
	}
	return _datafile;
}

bool Storage::cleanDatafile() {
	if (ThFS::exists(STORAGEKEY_DATAFILE))
		if (!ThFS::remove(STORAGEKEY_DATAFILE)) {
			DLOGLN("Couldn't remove datafile!");
			return false;
		}

	File f = ThFS::openW(STORAGEKEY_DATAFILE);
	if (!f) {
		DLOGLN("Couldn't open datafile!");
		return false;
	}
	
	_datafile.data.clear();

	return _config.writeToFile(f);
	// return ThFS::writeStruct(f, _datafile);
}

bool Storage::addMeasurementData(uint8_t val) {
	if (!retreiveDatafile())
		return false;
	_datafile.data.push_back(val);
	
	File f = ThFS::openW(STORAGEKEY_DATAFILE);
	if (!f) {
		DLOGLN("Couldn't open datafile!");
		return false;
	}
	return _datafile.writeToFile(f);
	// return ThFS::writeStruct(f, _datafile);
}