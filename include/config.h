#ifndef CONFIG_H__
#define CONFIG_H__

#include "sdk.h"
#include "storage.h"
#include <MsgPack.h>

class ConfigurationManager : public SetupBase {
    enum Config {
        HARDWARE,       // <bool> if hardware enabled
        WEBSERVER,      // <bool> if webserver enabled
    };

    StorageManager* _storage{ nullptr }; // TODO: use smart pointers
    MsgPack::map_t<uint8_t, uint8_t> _config;

    void createDefaultConfig();
public:
    bool setup(StorageManager* storage);
};

bool ConfigurationManager::setup(StorageManager* storage) {
    SetupBase::setup();

    _storage = storage;
    if (!_storage || !_storage->isReady()) {
        DLOGLN(F("StorageManager is not initialized!"));
        return false;
    }

    std::vector<uint8_t> data;
    if (_storage->exists(STORAGEKEY_CONFIG)) {
        DLOG(F("Found configuration file: '"));
        LOG(STORAGEKEY_CONFIG);
        LOGLN(F("'"));

        if (!_storage->read(STORAGEKEY_CONFIG, data)) {
            DLOGLN(F("Error loading configuration!"));
            return false;
        }
    } else { // config doesn't exist in storage yet
        DLOG(F("Configuration file: '"));
        LOG(STORAGEKEY_CONFIG);
        LOGLN(F("' not found!"));

        createDefaultConfig();

        MsgPack::Packer packer;
        packer.serialize(_config);
        DLOGLN(F("Created config: "));
        for (uint8_t i = 0; i < packer.size(); ++i) { LOG(packer.data()[i]); LOG(F(" ")); }
        LOGLN();

        if (!_storage->write(STORAGEKEY_CONFIG, packer.data(), packer.size())) {
            DLOGLN(F("Error writing configuration!"));
            return false;
        } else {
            DLOGLN(F("Configuration written!"));
        }
        data.assign(packer.data(), packer.data() + packer.size());
    }
    DLOG(F("Retrieved config: "));
    for (uint8_t b : data) { LOG(b); LOG(F(" ")); }
    LOGLN();

    MsgPack::Unpacker unpacker;
    unpacker.feed(data.data(), data.size());
    unpacker.deserialize(_config);

    DLOG(F("Parsed "));
    LOG(_config.size());
    LOGLN(F(" config entries:"));
    for (auto entry : _config) { LOG(entry.first); LOG(F(" -> ")); LOGLN(entry.second); }

    return _storage;
}

void ConfigurationManager::createDefaultConfig() {
    _config.insert({ ConfigurationManager::Config::HARDWARE, true });
    _config.insert({ ConfigurationManager::Config::WEBSERVER, false });
}

#endif // CONFIG_H__