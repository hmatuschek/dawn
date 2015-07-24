#include "temp.h"

#include <avr/io.h>

void temp_init() {
  ADCSRA = (1<<ADEN) | (0 << ADSC) | (0 << ADATE) | (0 << ADIF) | (0 << ADIE) |
      (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}


uint16_t
temp_get_core() {
  // Select ADC source
  ADMUX =
      // external reference (AREF=1.1V internal)
      (1 << REFS1) | (1 << REFS0) |
      // right adujst ADC result (LSB = bit0)
      (0 << ADLAR) |
      // Select temperature sensor
      (1 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
  // Start temperature conversion
  ADCSRA |= (1 << ADSC);
  // Wait for ADC finish
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

uint16_t
temp_get_external() {
  // Select ADC source
  ADMUX =
      // external reference (AREF=5V)
      (0 << REFS1) | (0 << REFS0) |
      // right adujst ADC result (LSB = bit0)
      (0 << ADLAR) |
      // Select ADC0 intput
      (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
  // Start temperature conversion
  ADCSRA |= (1 << ADSC);
  // Wait for ADC finish
  while (ADCSRA & (1 << ADSC)) {}
  return ADC;
}

