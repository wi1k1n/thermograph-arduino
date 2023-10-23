#ifndef SENSOR_H__
#define SENSOR_H__

#include "sdk.h"
#include <memory>

struct SensorData
{
	size_t timestamp = 0;
};

class Sensor {
private:
	SensorData _data;
public:
	Sensor() = default;
	virtual bool init() { return true; }
	virtual bool measure() { _data.timestamp = millis(); return true; }
	virtual bool isReady() const { return false; }
	virtual SensorData* getMeasurement() { return &_data; }
	virtual SensorData* waitForMeasurement(uint16_t timeout = 1000);
};

struct TempSensorData : public SensorData {
	float temp = 0;
};

class TempSensor : public Sensor {
private:
	TempSensorData _data;
public:
	TempSensor() = default;
	bool init() override;
	bool measure() override;
	bool isReady() const override;
	SensorData* getMeasurement() override;
};

#endif // SENSOR_H__