#ifndef TEMP_SENSOR
#define TEMP_SENSOR

#include "DS18.h"

class Sensor {
  public:
    Sensor(pin_t pin);
    void init();
    uint16_t getReadTime();
    bool requestRead();
    double getTemp(bool = false);
    void printDebugInfo();
  private: 
    pin_t _pin;
    DS18* _sensor;
    double _lastValue;
    uint8_t _address[8];
};

#endif
