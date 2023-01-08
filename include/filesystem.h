#ifndef FILESYSTEM_H__
#define FILESYSTEM_H__

#include "LittleFS.h"

struct Config {

};

class AutoFile : public File {
	File _file;
public:
	AutoFile(File f) : _file(f) { }
	~AutoFile() { _file.close(); }
};

class Storage {
public:
	static bool init();
	
	static bool isSleeping();
private:
	static Config _config;

    static AutoFile open(const char* path, const char* mode);
    static AutoFile open(const String& path, const char* mode);
	static AutoFile openR(const char* path);
	static AutoFile openW(const char* path);
};

#endif // FILESYSTEM_H__