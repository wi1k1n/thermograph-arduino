#ifndef SENSOR_H__
#define SENSOR_H__

#include <Arduino.h>
#include "sdk.h"

class SensorInterface {
public:
    virtual void Init();
    virtual void GetMeasurement(void* measurement);
};

class AnalogInputSensorInterface : public SensorInterface {

};



class TempSensor : public SensorInterface {
    Float _temperature;
public:
    virtual void GetMeasurement(void* measurement) {
        Float meas = _temperature;
        measurement = static_cast<void*>(&meas);
    }
};

class LightSensor : public SensorInterface {
};

class Thermistor : public TempSensor, public AnalogInputSensorInterface {
    static const Double C1 = 1.009249522e-03;  // Steinhart–Hart equation (for thermistor) constants
    static const Double C2 = 2.378405444e-04;
    static const Double C3 = 2.019202697e-07;
    #define R1 10000  // value of pull up resistor connected to thermistor

    // converts 0..1023 value from analog input to degrees of Celsius
    Float computeTemp(const Float &analogValue) {
        // Constants and formulas are from:
        // https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
        // Nice mini-class for thermistor is from:
        // https://github.com/AlexGyver/GyverLibs/tree/master/minimLibs/thermistor
        // Wiring:
        // GND --- thermistor --- A0 --- 10k --- 5V
        Float logR2 = log(R1 * (1023.f / analogValue - 1.f));
        return (((1.f / (C1 + C2*logR2 + C3*logR2*logR2*logR2)) - 273.15f));
    }
};

#endif // SENSOR_H__