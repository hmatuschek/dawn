/** Provides access to the temperature sensors, the internal one
 * present in ATMegaX8P/A/PA MCUs and an external one connected to ADC0. */

#ifndef __DAWN_TEMP_H__
#define __DAWN_TEMP_H__

#include <inttypes.h>

/** Initializes the ADC. */
void temp_init();
/** Reads the internal temp sensor. */
uint16_t temp_get_core();
/** Reads the external temp sensor. */
uint16_t temp_get_external();

#endif // __DAWN_TEMP_H__
