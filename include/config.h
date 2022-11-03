#ifndef CONFIG_H__
#define CONFIG_H__

#include <sdk.h>

class ConfigurationManager {
public:
    Bool setup();
};

Bool ConfigurationManager::setup() {
    return true;
}

#endif // CONFIG_H__