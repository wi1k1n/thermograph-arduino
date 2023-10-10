#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <Arduino.h>

class Application;

class ThSettings {
public:
	enum class Entries {
		PERIOD = 0,
		N_MEASUREMENTS
	};
public:
	ThSettings() = default;

	bool init(Application* app);

	template<typename T>
	const T& getEntry(const Entries& entry) const;
	template<typename T>
	bool setEntry(const Entries& entry, const T& val);
	template<typename T>
	T getEntryDefault(const Entries& entry) const;

	bool storeConfig();
private:
	Application* _app = nullptr;
	
	uint16_t _periodMeasurement = 600; // in seconds
	uint16_t _numberOfMeasurements = 144;
};

#endif // SETTINGS_H__