#ifndef SETTINGS_H__
#define SETTINGS_H__

#include <Arduino.h>

class Application;

class ThSettings {
public:
	enum class Entries {
		PERIOD_CAPTURE = 0, 	// uint16_t, in seconds; // TODO: change for more compact way of storing the index
		N_MEASUREMENTS, 		// uint16_t
		PERIOD_LIVE 			// uint16_t, in ms
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

	uint16_t _periodCaptureMeasurement = 600; 	// in seconds
	uint16_t _numberOfMeasurements = 144;
	uint16_t _periodLiveMeasurement = 1000; 	// in ms
};

#endif // SETTINGS_H__