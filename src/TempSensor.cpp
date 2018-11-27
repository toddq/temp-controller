#include "TempSensor.h"

DS18 *sensor;

void initSensor(uint16_t pin) {
  Serial.printlnf("Initializing sensor pin %d", pin);
  sensor = new DS18(pin);
}

// the library iterates over addresses on the 1-wire bus
double getTemp() {
  if (sensor->read()) {
    return sensor->fahrenheit();
  } else {
    if (sensor->searchDone()) {
      return getTemp();
    } else {
      Serial.print("Read failed");
      printDS18b20DebugInfo();
    }
  }
}

void printDS18b20DebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (sensor->crcError()) {
    Serial.print("CRC Error ");
  }

  // Print the sensor type
  const char *type;
  switch(sensor->type()) {
    case WIRE_DS1820: type = "DS1820"; break;
    case WIRE_DS18B20: type = "DS18B20"; break;
    case WIRE_DS1822: type = "DS1822"; break;
    case WIRE_DS2438: type = "DS2438"; break;
    default: type = "UNKNOWN"; break;
  }
  Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  sensor->addr(addr);
  Serial.printf(
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  );

  // Print the raw sensor data
  uint8_t data[9];
  sensor->data(data);
  Serial.printf(
    " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  );
}
