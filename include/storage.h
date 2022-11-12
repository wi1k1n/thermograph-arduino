#ifndef STORAGE_H__
#define STORAGE_H__

#include <sdk.h>
#include "LittleFS.h"

class StorageManager : public SetupBase {
public:
    bool setup() override;

    bool exists(const String& key);
    bool write(const String& key, const uint8_t* buffer, size_t length, bool create = true);
    bool read(const String& key, std::vector<uint8_t>& dst);
};

bool StorageManager::setup() {
    SetupBase::setup();
    return LittleFS.begin();
}

bool StorageManager::exists(const String& key) {
    return LittleFS.exists(F("/") + key);
}

bool StorageManager::write(const String& key, const uint8_t *buffer, size_t length, bool create) {
    if (!create && !LittleFS.exists(F("/") + key)) {
        return false;
    }
    DLOGLN(F("Writing data"));
    File f = LittleFS.open(F("/") + key, "w");
    f.write(buffer, length);
    f.close();
    return true;
}

bool StorageManager::read(const String& key, std::vector<uint8_t>& dst) {
    if (!LittleFS.exists(key)) {
        return false;
    }
    DLOGLN(F("Reading data"));
    File f = LittleFS.open(F("/") + key, "rb");
    dst.resize(f.size());
    f.read(dst.data(), f.size());
    f.close();
    return true;
}

#endif // STORAGE_H__