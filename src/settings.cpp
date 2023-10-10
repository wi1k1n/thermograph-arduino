#include "settings.h"
#include "filesystem.h"

template<> const uint16_t& ThSettings::getEntry(const Entries& entry) const {
	if (entry == Entries::PERIOD)
		return _periodMeasurement;
	if (entry == Entries::N_MEASUREMENTS)
		return _numberOfMeasurements;
	return 0;
}

template<> uint16_t ThSettings::getEntryDefault(const Entries& entry) const {
	if (entry == Entries::PERIOD)
		return 300;
	if (entry == Entries::N_MEASUREMENTS)
		return 144;
	return 0;
}

bool ThSettings::init(Application* app) {
	if (!app)
		return false;
	_app = app;

	SStrConfig& config = Storage::getConfig();
	if (!config.period || !config.nMeasurements) {
		_periodMeasurement = getEntryDefault<uint16_t>(Entries::PERIOD);
		_numberOfMeasurements = getEntryDefault<uint16_t>(Entries::N_MEASUREMENTS);
		if (!storeConfig())
			return false;
	} else {
		_periodMeasurement = config.period;
		_numberOfMeasurements = config.nMeasurements;
	}
	return true;
}

bool ThSettings::storeConfig() {
	SStrConfig& config = Storage::getConfig();
	config.period = getEntry<uint16_t>(Entries::PERIOD);
	config.nMeasurements = getEntry<uint16_t>(Entries::N_MEASUREMENTS);
	if (!Storage::storeConfig())
		return false;
	return true;
}

template<> bool ThSettings::setEntry(const Entries& entry, const uint16_t& val) {
	if (entry == Entries::PERIOD) {
		_periodMeasurement = val;
		_app->setTimerMeasurementPeriod(_periodMeasurement);
		return true;
	}
	if (entry == Entries::N_MEASUREMENTS) {
		_numberOfMeasurements = val;
		return true;
	}
	return false;
}