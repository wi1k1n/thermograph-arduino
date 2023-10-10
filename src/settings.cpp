#include "settings.h"
#include "filesystem.h"

template<> const uint16_t& ThSettings::getEntry(const Entries& entry) const {
	if (entry == Entries::PERIOD_CAPTURE)
		return _periodCaptureMeasurement;
	if (entry == Entries::N_MEASUREMENTS)
		return _numberOfMeasurements;
	if (entry == Entries::PERIOD_LIVE)
		return _periodLiveMeasurement;
	return 0;
}

template<> uint16_t ThSettings::getEntryDefault(const Entries& entry) const {
	if (entry == Entries::PERIOD_CAPTURE)
		return 300;
	if (entry == Entries::N_MEASUREMENTS)
		return 144;
	if (entry == Entries::PERIOD_LIVE)
		return 1000;
	return 0;
}

bool ThSettings::init(Application* app) {
	if (!app)
		return false;
	_app = app;

	SStrConfig& config = Storage::getConfig();
	if (!config.periodCapture || !config.nMeasurements || !config.periodLive) {
		_periodCaptureMeasurement = getEntryDefault<uint16_t>(Entries::PERIOD_CAPTURE);
		_numberOfMeasurements = getEntryDefault<uint16_t>(Entries::N_MEASUREMENTS);
		_periodLiveMeasurement = getEntryDefault<uint16_t>(Entries::PERIOD_LIVE);
		if (!storeConfig())
			return false;
	} else {
		_periodCaptureMeasurement = config.periodCapture;
		_numberOfMeasurements = config.nMeasurements;
		_periodLiveMeasurement = config.periodLive;
	}
	return true;
}

bool ThSettings::storeConfig() {
	SStrConfig& config = Storage::getConfig();
	config.periodCapture = getEntry<uint16_t>(Entries::PERIOD_CAPTURE);
	config.nMeasurements = getEntry<uint16_t>(Entries::N_MEASUREMENTS);
	config.periodLive = getEntry<uint16_t>(Entries::PERIOD_LIVE);
	if (!Storage::storeConfig())
		return false;
	return true;
}

template<> bool ThSettings::setEntry(const Entries& entry, const uint16_t& val) {
	if (entry == Entries::PERIOD_CAPTURE) {
		_periodCaptureMeasurement = val;
		return true;
	}
	if (entry == Entries::N_MEASUREMENTS) {
		_numberOfMeasurements = val;
		return true;
	}
	if (entry == Entries::PERIOD_LIVE) {
		_periodLiveMeasurement = val;
		_app->setRealtimeMeasurementPeriod(_periodLiveMeasurement);
		return true;
	}
	return false;
}