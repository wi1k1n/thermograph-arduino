#include "sensor.h"
#include "sdk.h"
#include <exception>

// wait for measurement to be ready in timeout (in ms)
// return nullptr if timeouted
SensorData* Sensor::waitForMeasurement(uint16_t timeout) {
    long startTime = millis();
    while (!isReady()) {
        if (millis() - startTime >= timeout)
            return nullptr;
    }
    return getMeasurement();
}

// initialization routine
bool TempSensor::init() {
    return true;
}

// starts measurement, returns true if successfully started, false if there was an error
bool TempSensor::measure() {
    // _data.temp = 123;
    _data.temp = (int)analogRead(A0);
    return true;
}

bool TempSensor::isReady() const {
    return true;
}

SensorData* TempSensor::getMeasurement() {
    return static_cast<SensorData*>(&_data);
}