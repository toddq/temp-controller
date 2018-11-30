#include "TempSensor.h"

Sensor::Sensor(uint16_t pin) {
  _pin = pin;
}

void Sensor::init() {
  Serial.printlnf("Initializing sensor pin %d", _pin);
  _sensor = new DS18(_pin);
  // start with an immediate read so we can save the address and skip
  // the 1wire search on every subsequent read
  if (_sensor->read()) {
    _lastValue = _sensor->fahrenheit();
    _sensor->addr(_address);
    Serial.print("Got sensor address - ");
    Serial.printlnf(" ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
      _address[0], _address[1], _address[2], _address[3], _address[4], _address[5], _address[6], _address[7]
    );
  }
}

uint16_t Sensor::getReadTime() {
  return _sensor->getConversionTime();
}

bool Sensor::requestRead() {
  return _sensor->requestRead(_address);
}

double Sensor::getTemp(bool blockingCall) {
  if (blockingCall) {
    if (_sensor->read(_address)) {
      _lastValue = _sensor->fahrenheit();
    } else {
      printDebugInfo();
    }
  } else {
    if (_sensor->readTemp()) {
      _lastValue = _sensor->fahrenheit();
    } else {
      printDebugInfo();
    }
  }
  return _lastValue;
}

void Sensor::printDebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (_sensor->crcError()) {
    Serial.print("CRC Error ");
  }

  // Print the sensor type
  const char *type;
  switch(_sensor->type()) {
    case WIRE_DS1820: type = "DS1820"; break;
    case WIRE_DS18B20: type = "DS18B20"; break;
    case WIRE_DS1822: type = "DS1822"; break;
    case WIRE_DS2438: type = "DS2438"; break;
    default: type = "UNKNOWN"; break;
  }
  Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  _sensor->addr(addr);
  Serial.printf(
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  );

  // Print the raw sensor data
  uint8_t data[9];
  _sensor->data(data);
  Serial.printlnf(
    " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  );
}
