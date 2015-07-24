#include "pwm.h"
#include <avr/io.h>

void
pwm_init() {
  // PB1 is now an output
  DDRB |= (1 << DDB1);
  // set value to 0
  OCR1A = 0x0000;
  // set none-inverting mode
  TCCR1A |= (1 << COM1A1);
  // set Fast PWM mode using ICR1 as TOP
  TCCR1A |= (1 << WGM11) | (1 << WGM10);
  //TCCR1B |= (1 << WGM12) | (1 << WGM13);
  // START the timer with no prescaler
  TCCR1B |= (1 << CS11);
}


void
pwm_set(uint16_t value) {
  OCR1A = value;
}
