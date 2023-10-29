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

	Storage::readConfig();
	SStrConfig& config = Storage::getConfig();
	if (!config.isValid()) {
		DLOGLN("Config is invalid, creating default one..");
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
	if (!Storage::writeConfig())
		return false;
	DLOG("Config stored! Values: "); LOG(config.periodCapture); LOG(" | "); LOG(config.nMeasurements); LOG(" | "); LOGLN(config.periodLive);
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