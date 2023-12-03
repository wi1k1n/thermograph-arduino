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
}
bool SStrDatafile::convertFromVersion(uint8_t vMaj, uint8_t vMin) {
	return false; // TODO: implement
}

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



bool Storage::readDatafile() {
	if (!ThFS::exists(STORAGEKEY_DATAFILE))
		return false;
	return ThFS::readStruct(STORAGEKEY_DATAFILE, _datafile);
}

bool Storage::writeDatafileAndCleanContainer() {
	removeDatafileAndContainer();
	
	if (!ThFS::openW(STORAGEKEY_DATAFILE_CONTAINER)) {
		DLOGLN("Couldn't create empty dataFileContainer in the filesystem!");
		return false;
	}

	return ThFS::writeStruct(STORAGEKEY_DATAFILE, _datafile);
}

bool Storage::removeDatafileAndContainer() {
	if (ThFS::exists(STORAGEKEY_DATAFILE_CONTAINER)) {
		if (!ThFS::remove(STORAGEKEY_DATAFILE_CONTAINER)) {
			DLOGLN("Couldn't remove existing dataFile in the filesystem!");
			return false;
		}
	}
	if (ThFS::exists(STORAGEKEY_DATAFILE)) {
		if (!ThFS::remove(STORAGEKEY_DATAFILE)) {
			DLOGLN("Couldn't remove existing dataFile in the filesystem!");
			return false;
		}
	}
	return true;
}

bool Storage::addMeasurementData(uint8_t val) {
	if (!ThFS::exists(STORAGEKEY_DATAFILE_CONTAINER)) {
		DLOGLN("WARDNING: no dataFileContainer found when adding measurement. New file will be created!");
		if (!ThFS::openW(STORAGEKEY_DATAFILE_CONTAINER)) {
			DLOGLN("Couldn't create empty dataFileContainer in the filesystem!");
			return false;
		}
	}

	File f = ThFS::openA(STORAGEKEY_DATAFILE_CONTAINER);
	if (!f) {
		DLOGLN("Couldn't open dataFileContainer for appending!");
		return false;
	}

	f.write(val); // TODO: consider bitsPerMeasurement and other configs when writing at this point
	
	return true;
}

bool Storage::readData(std::vector<uint8_t>& dst, size_t offset, size_t count) {
	File f = ThFS::openR(STORAGEKEY_DATAFILE_CONTAINER);
	if (!f) {
		DLOGLN("Couldn't open dataFileContainer for reading!");
		return false;
	}
	// TODO: Here bitsPerMeasurement and other configs should be considered!

	uint16_t availCount = f.available();
	if (availCount <= offset || !f.seek(offset)) {
		DLOGLN("There's less data than tried to offset on reading!");
		return false;
	}
	
	availCount = f.available();
	if (!count)
		count = availCount;
	
	if (availCount < count) {
		DLOGLN("There's not enough data to read!");
		return false;
	}

	dst.resize(count);
	uint8_t* buff = dst.data();
	if (f.readBytes((char*)buff, count) != count) {
		DLOGLN("Error reading data from dataFileContainer");
		return false;
	}
	return true;
}