#ifndef SENSOR_H__
#define SENSOR_H__

#include "sdk.h"
#include "TimerMs.h"
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
	virtual void tick() { }
	virtual bool measure() { _data.timestamp = millis(); return true; }
	virtual bool isReady() const { return false; }
	virtual SensorData* getMeasurement() { return &_data; }
	virtual SensorData* waitForMeasurement(uint16_t timeout = 1000);
};

struct TempSensorData : public SensorData {
	float temp = 0;
};

class TempSensor : public Sensor {
protected:
	TempSensorData _data;
public:
	TempSensor() = default;
	bool init() override;
	bool measure() override;
	bool isReady() const override;
	SensorData* getMeasurement() override;
};

class NTCThermistor : public TempSensor {
public:
	bool init(uint8_t adcPin, double r1);
	void tick() override;
	bool measure() override;
	bool isReady() const override;
private:
	float calculateTemp(uint16_t adcReading) const;
private:
	// Steinhart-Hart equations consts
	const double C1 = 1.009249522e-03;
	const double C2 = 2.378405444e-04;
	const double C3 = 2.019202697e-07;


	uint8_t _adcPin = 255;
	double _r1 = 0;

	const uint8_t AVERAGING_MEASUREMENTS = 10;
	const uint8_t MEASUREMENT_DURATION = 50; // ms
	TimerMs _timerAveraging;
	uint8_t _measurementsCount = 0;
	uint16_t _cumulativeADCMeasurements = 0;
};

#endif // SENSOR_H__