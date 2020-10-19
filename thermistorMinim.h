// мини-класс для работы с термисторами по закону Стейнхарта-Харта
// GND --- термистор --- A0 --- 10к --- 5V
// https://github.com/AlexGyver/GyverLibs/tree/master/minimLibs/thermistor

// constants for Steinhart–Hart equation
#define C1 1.009249522e-03
#define C2 2.378405444e-04
#define C3 2.019202697e-07

class thermistor {
  public:
    thermistor(byte pin, int resistance);
    float computeTemp(float analogValue);
  private:
    byte _pin = 0;
    int _resistance = 10000;
};

thermistor::thermistor(byte pin, int resistance) {
  _pin = pin;
  _resistance = resistance;
}

// converts 0..1023 value from analog input to degrees of Celsius
float thermistor::computeTemp(float analogValue) {
  // https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
  float logR2 = log(_resistance * (1023.f / analogValue - 1.f));
  return (1.f / (C1 + C2*logR2 + C3*logR2*logR2*logR2)) - 273.15f;
}