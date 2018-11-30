#ifndef TEMP_SENSOR
#define TEMP_SENSOR

#include "DS18.h"

class Sensor {
  public:
    Sensor(uint16_t pin);
    void init();
    uint16_t getReadTime();
    bool requestRead();
    double getTemp(bool = false);
    void printDebugInfo();
  private: 
    uint16_t _pin;
    DS18* _sensor;
    double _lastValue;
    uint8_t _address[8];
};

#endif
