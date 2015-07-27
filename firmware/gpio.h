#ifndef __DAWN_GPIO_H__
#define __DAWN_GPIO_H__

#include <inttypes.h>

typedef enum {
  KEY_NONE  = 0,
  KEY_KLICK = 1,
  KEY_HOLD  = 2
} KeyState;

// Initializes the capacitive touch "sensors".
void touch_init();
// Touch sensor readout (NONE, KLICK, HOLD)
KeyState touch_pin(uint8_t key);
// Update keys.
void touch_update_keys();


#endif // __DAWN_GPIO_H__
