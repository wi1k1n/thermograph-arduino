#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "LittleFS.h"

struct Config {

};
class Storage {
public:
	static bool init();
	
	static bool isSleeping();
private:
	static Config _config;
};

#endif // FILESYSTEM_H__