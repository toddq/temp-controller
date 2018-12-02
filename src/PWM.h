#ifndef PWM_CONTROL
#define PWM_CONTROL

#include "Particle.h"

// low and slow pwm
class PWMControl {
  public:
    PWMControl(pin_t pin, unsigned long frequency = 1000);
    void process();
    void setDutyCycle(int percent);
    void setFrequency(long frequency);
    void printDebug();

  private:
    void power(bool newState);
    pin_t _outputPin;
    unsigned long _frequency;
    unsigned long _dutyCycle;
    unsigned long _cycleStart;
    bool _powerState;
};

#endif
