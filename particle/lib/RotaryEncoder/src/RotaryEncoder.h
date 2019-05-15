#ifndef rotary_encoder_h
#define rotary_encoder_h

#include "Particle.h"

class RotaryEncoder {
    public:
        RotaryEncoder(int, int);
        void init(raw_interrupt_handler_t);
        int process();
    private:
        int pin1;
        int pin2;
        volatile bool A_set;
        volatile bool B_set;
};

#endif
