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

template <typename T> bool ThFS::readStruct(File& f, T& s) {
	static_assert(std::is_base_of_v<StorageStruct, T>);
	
	int availableBytes = f.available();
	size_t byteCount = sizeof(T);
	if ((int)byteCount > availableBytes) // not enough bytes available
		return false;

	char buffer[byteCount];
	size_t byteCountRead = f.readBytes(buffer, byteCount);

	if (byteCountRead != byteCount) // couldn't read enough bytes
		return false;
	
	s.copyFrom(T(*reinterpret_cast<T*>(buffer)));
	return true;
}

template <typename T>
bool ThFS::readStruct(const char* path, T& s) {
	File f = ThFS::openR(path);
	if (!f) {
		DLOGLN("Couldn't open file for reading!");
		return false;
	}
	return ThFS::readStruct(f, s);
}

template <typename T>
bool ThFS::writeStruct(File& f, T& s) {
	static_assert(std::is_base_of_v<StorageStruct, T>);
	
	size_t byteCount = sizeof(T);
	size_t byteCountWritten = f.write(reinterpret_cast<char*>(&s), byteCount);
	return byteCountWritten == byteCount;
}

template <typename T>
bool ThFS::writeStruct(const char* path, T& s) {
	File f = ThFS::openW(path);
	if (!f) {
		DLOGLN("Couldn't open file for writing!");
		return false;
	}
	return ThFS::writeStruct(f, s);
}

/// ////////////////////////////////////

// TODO: what a disgusting coding pattern here :-(
// TODO: cow pattern?
void SStrSleeping::copyFrom(const SStrSleeping& other) {
	StorageStruct::copyFrom(other);
	timeAwake = other.timeAwake;
	mode = other.mode;
}
bool SStrSleeping::convertFromVersion(uint8_t vMaj, uint8_t vMin) {
	return false; // TODO: implement
}

void SStrConfig::copyFrom(const SStrConfig& other) {
	StorageStruct::copyFrom(other);
	periodCapture = other.periodCapture;
	nMeasurements = other.nMeasurements;
	periodLive = other.periodLive;
}
bool SStrConfig::convertFromVersion(uint8_t vMaj, uint8_t vMin) {
	return false; // TODO: implement
}

void SStrDatafile::copyFrom(const SStrDatafile& other) {
	StorageStruct::copyFrom(other);
	data = other.data;
}
bool SStrDatafile::convertFromVersion(uint8_t vMaj, uint8_t vMin) {
	return false; // TODO: implement
}


// int16_t SStrDatafile::readFromFile(const char* path) {
// 	// int16_t offset = StorageStruct::readFromFile(path);
// 	// File f = ThFS::openR(path);
// 	// if (!f) {
// 	// 	DLOGLN("Couldn't open datafile for reading!");
// 	// 	return false;
// 	// }
// 	// f.seek(offset);
	
// 	// char buffer[12];
// 	// if (f.readBytes(buffer, sizeof(uint16_t)) != sizeof(uint16_t)) {
// 	// 	DLOGLN("Error reading data length!");
// 	// 	return false;
// 	// }
	
// 	// uint16_t dSize;
// 	// memcpy(&dSize, buffer, sizeof(uint16_t));
// 	// DLOG("Reading datafile: ["); LOG(dSize); LOG("]");

// 	// if (!dSize)
// 	// 	return true;
	
// 	// data.resize(dSize);
// 	// for (uint16_t i = 0; i < dSize; ++i) {
// 	// 	if (f.readBytes(buffer, sizeof(uint8_t)) != sizeof(uint8_t)) {
// 	// 		DLOGLN("Error reading data!");
// 	// 		return false;
// 	// 	}
// 	// 	memcpy(&data[i], buffer, sizeof(uint8_t));
// 	// 	LOG(" ");
// 	// 	LOG(data[i]);
// 	// }

// 	return true;
// }
// int16_t SStrDatafile::writeToFile(const char* path) {
// 	// StorageStruct::writeToFile(path);
// 	// File f = ThFS::openA(path);
// 	// if (!f) {
// 	// 	DLOGLN("Couldn't open datafile for writing!");
// 	// 	return false;
// 	// }

// 	// f.write(static_cast<uint16_t>(data.size()));
// 	// for (auto v : data) {
// 	// 	f.write(v);
// 	// }
// 	return true;
// }

SStrConfig Storage::_config;
SStrSleeping Storage::_sleeping;
SStrDatafile Storage::_datafile;

/// ////////////////////////////////////

bool Storage::init() {
	if (!ThFS::init())
		return false;
	return true;
}


bool Storage::readSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING))
		return false;
	return ThFS::readStruct(STORAGEKEY_ISSLEEPING, _sleeping);
}

bool Storage::writeSleeping() {
	if (!ThFS::exists(STORAGEKEY_ISSLEEPING)) {
		if (!ThFS::openW(STORAGEKEY_ISSLEEPING)) {
			DLOGLN("Couldn't create isSleeping file in the filesystem!");
			return false;
		}
	}
	return ThFS::writeStruct(STORAGEKEY_ISSLEEPING, _sleeping);
}

bool Storage::removeSleeping() {
	if (ThFS::exists(STORAGEKEY_ISSLEEPING))
		return ThFS::remove(STORAGEKEY_ISSLEEPING);
	return true;
}



bool Storage::readConfig() {
	if (!ThFS::exists(STORAGEKEY_CONFIG))
		return false;
	return ThFS::readStruct(STORAGEKEY_CONFIG, _config);
}

bool Storage::writeConfig() {
	if (!ThFS::exists(STORAGEKEY_CONFIG)) {
		if (!ThFS::openW(STORAGEKEY_CONFIG)) {
			DLOGLN("Couldn't create config file in the filesystem!");
			return false;
		}
	}
	return ThFS::writeStruct(STORAGEKEY_CONFIG, _config);
}

bool Storage::removeConfig() {
	if (ThFS::exists(STORAGEKEY_CONFIG))
		return ThFS::remove(STORAGEKEY_CONFIG);
	return true;
}


// bool Storage::retreiveConfig(bool createNew) {
// 	bool exists = ThFS::exists(STORAGEKEY_CONFIG);
// 	if (exists) {
// 		bool success = _config.readFromFile(STORAGEKEY_CONFIG);
// 		DLOG("Config retrieved! Values: "); LOG(_config.periodCapture); LOG(" | "); LOG(_config.nMeasurements); LOG(" | "); LOGLN(_config.periodLive);
// 		return success;
// 	}
// 	// DLOGLN("doesn't exists");
// 	if (!createNew)
// 		return false;
// 	return _config.writeToFile(STORAGEKEY_CONFIG);
// }

// SStrConfig& Storage::getConfig(bool retrieve) {
// 	if (retrieve) {
// 		retreiveConfig();
// 	}
// 	return _config;
// }

// bool Storage::storeConfig() {
// 	return _config.writeToFile(STORAGEKEY_CONFIG);
// }

bool Storage::retreiveDatafile(bool createNew) {
	// bool exists = ThFS::exists(STORAGEKEY_DATAFILE);
	// if (exists)
	// 	return _datafile.readFromFile(STORAGEKEY_DATAFILE);
	// if (!createNew)
	// 	return false;
	// return _datafile.writeToFile(STORAGEKEY_DATAFILE);
	return true;
}

SStrDatafile& Storage::getDatafile(bool retrieve) {
	if (retrieve) {
		retreiveDatafile();
	}
	return _datafile;
}

bool Storage::cleanDatafile() {
	// if (ThFS::exists(STORAGEKEY_DATAFILE))
	// 	if (!ThFS::remove(STORAGEKEY_DATAFILE)) {
	// 		DLOGLN("Couldn't remove datafile!");
	// 		return false;
	// 	}
	
	// _datafile.data.clear();

	// return _config.writeToFile(STORAGEKEY_DATAFILE);
	return true;
}

bool Storage::addMeasurementData(uint8_t val) {
	// if (!retreiveDatafile())
	// 	return false;

	// _datafile.data.push_back(val);

	// return _datafile.writeToFile(STORAGEKEY_DATAFILE);
	return true;
}