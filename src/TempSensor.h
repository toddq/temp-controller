#ifndef TEMP_SENSOR
#define TEMP_SENSOR

#include "DS18.h"

void initSensor(uint16_t pin);
double getTemp();
void printDS18b20DebugInfo();

#endif
