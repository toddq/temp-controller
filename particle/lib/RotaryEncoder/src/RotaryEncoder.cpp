#include "Particle.h"
#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(int _pin1, int _pin2) {
    pin1 = _pin1;
    pin2 = _pin2;
    A_set = true;
    B_set = false;
}

void RotaryEncoder::init(raw_interrupt_handler_t handler) {
  Serial.printlnf("RotaryEncoder::init  %d, %d", pin1, pin2);
  // Set pins to input.
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);

  attachInterrupt(pin1, handler, CHANGE);
  attachInterrupt(pin2, handler, CHANGE);
}

int RotaryEncoder::process() {
  if( digitalRead(pin1) != A_set ) {  // debounce once more
    A_set = !A_set;
    if ( A_set && !B_set )
      return 1;
  } else if( digitalRead(pin2) != B_set ) {
    B_set = !B_set;
    if( B_set && !A_set )
      return -1;
  } else {
  }
  return 0;
}
