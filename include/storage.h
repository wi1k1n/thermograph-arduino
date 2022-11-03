#ifndef STORAGE_H__
#define STORAGE_H__

#include <sdk.h>
#include "LittleFS.h"

class StorageManager {
public:
    Bool setup();
};

Bool StorageManager::setup() {
    return LittleFS.begin();
}

#endif // STORAGE_H__