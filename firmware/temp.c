#include "temp.h"

#include <avr/io.h>

void temp_init() {
  ADCSRA =
      // Enable the ADC
      (1 << ADEN) |
      // Do not start conversion yet
      (0 << ADSC) |
      // No conversion trigger
      (0 << ADATE) |
      // No ADC interrupt
      (0 << ADIF) | (0 << ADIE) |
      // ADC Prescaler - 16 (16MHz -> 1MHz)
      (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
}


uint16_t
temp_get_core() {
  // Select ADC source
  ADMUX =
      // external reference (AREF=5V)
      (0 << REFS1) | (0 << REFS0) |
      // right adujst ADC result (LSB = bit0)
      (0 << ADLAR) |
      // Select temperature sensor
      (1 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
  // Start temperature conversion
  ADCSRA |= (1 << ADSC);
  // Wait for ADC finish
  while (ADCSRA && (1 << ADSC)) {}
  return ((uint16_t) ADCH)<<8 | ADCL;
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
  while (ADCSRA && (1 << ADSC)) {}
  return ((uint16_t) ADCH)<<8 | ADCL;
}

