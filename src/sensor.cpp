#include "sensor.h"
#include <exception>

// wait for measurement to be ready in timeout (in ms)
// return nullptr if timeouted
SensorData* Sensor::waitForMeasurement(uint16_t timeout) {
	long startTime = millis();
	while (!isReady()) {
		if (millis() - startTime >= timeout)
			return nullptr;
		tick();
	}
	return getMeasurement();
}

// initialization routine
bool TempSensor::init() {
	return true;
}

// starts measurement, returns true if successfully started, false if there was an error
bool TempSensor::measure() {
	if (!Sensor::measure())
		return false;
	return true;
}

bool TempSensor::isReady() const {
	return true;
}

SensorData* TempSensor::getMeasurement() {
	return static_cast<SensorData*>(&_data);
}


bool NTCThermistor::init(uint8_t adcPin, double r1) {
	_adcPin = adcPin;
	_r1 = r1;
	_timerAveraging.setTime((float)MEASUREMENT_DURATION / AVERAGING_MEASUREMENTS);

	return true;
}

float NTCThermistor::calculateTemp(uint16_t adcReading) const {
	float r2 = _r1 * (1023.0 / (float)adcReading - 1.0);
	float logR2 = log(r2);
	float Tk = 1.0 / (C1 + C2 * logR2 + C3 * logR2 * logR2 * logR2); // https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation
	return Tk - 273.15; // in celcius
}

bool NTCThermistor::measure() {
	if (!TempSensor::measure())
		return false;

	_measurementsCount = 1;
	_cumulativeADCMeasurements = analogRead(_adcPin);
	_timerAveraging.start();

	return true;
}

void NTCThermistor::tick() {
	if (_timerAveraging.tick()) {
		_measurementsCount++;
		_cumulativeADCMeasurements += analogRead(_adcPin);

		if (isReady()) {
			_timerAveraging.stop();
			// TODO: need proof that averaging along adc output is the same as averaging along temp values
			_data.temp = calculateTemp((float)_cumulativeADCMeasurements / _measurementsCount);
			_data.timestamp = millis();
		}
	}
}

bool NTCThermistor::isReady() const {
	return _measurementsCount >= AVERAGING_MEASUREMENTS;
}