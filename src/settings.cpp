#include "settings.h"

bool ThSettings::init() {
	return true;
}

template<> const uint16_t& ThSettings::getEntry(const Entries& entry) const {
	if (entry == Entries::PERIOD)
		return _periodMeasurement;
	if (entry == Entries::N_MEASUREMENTS)
		return _numberOfMeasurements;
	return 0;
}


template<> bool ThSettings::setEntry(const Entries& entry, const uint16_t& val) {
	if (entry == Entries::PERIOD) {
		_periodMeasurement = val;
		return true;
	}
	if (entry == Entries::N_MEASUREMENTS) {
		_numberOfMeasurements = val;
		return true;
	}
	return false;
}