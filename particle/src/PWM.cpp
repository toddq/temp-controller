#include "PWM.h"

PWMControl::PWMControl(pin_t ouputPin, unsigned long frequency) {
  _outputPin = ouputPin;
  _frequency = frequency;
  // default 100%
  _dutyCycle = _frequency;
}

void PWMControl::process() {
  unsigned long now = millis();
  unsigned long elapsedCycle = now - _cycleStart;
  if (_dutyCycle == _frequency) {
    // 100% == on
    power(true);
  } else if (_dutyCycle == 0) {
    // 0% == off
    power(false);
  } else 
  if (elapsedCycle > _frequency) {
    // start of a new cycle
    _cycleStart = now;
    power(true);
  } else {
    // hold steady
    power(elapsedCycle < _dutyCycle);
  }
}

void PWMControl::power(bool newState) {
  if (newState != _powerState) {
    _powerState = newState;
    digitalWrite(_outputPin, _powerState);
  }
}

void PWMControl::setDutyCycle(int percent) {
  percent = constrain(percent, 0, 100);
  _dutyCycle = _frequency * 0.01 * percent;
  // Serial.printlnf("Setting duty cycle to %d%% - %d ms", percent, _dutyCycle);
}

void PWMControl::setFrequency(long frequency) {
  float dutyCyclePercent = (float)_dutyCycle / (float)_frequency;
  _frequency = frequency;
  setDutyCycle((int)(dutyCyclePercent * 100));
}

void PWMControl::printDebug() {
  Serial.println("");
  Serial.println(Time.timeStr());
  float dutyCyclePercent = (float)_dutyCycle / (float)_frequency;
  Serial.printlnf("Frequency: %d,  dutyCycle: %d (%d%%)", _frequency, _dutyCycle, (int)(dutyCyclePercent * 100));
  Serial.printlnf("Cycle start: %d", _cycleStart);
  Serial.printlnf("Now: %d", millis());
  Serial.printlnf("Power state: %s", _powerState ? "on" : "off");
}
