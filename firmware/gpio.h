#ifndef __DAWN_GPIO_H__
#define __DAWN_GPIO_H__

#include <inttypes.h>

typedef enum {
  KEY_NONE  = 0,
  KEY_KLICK = 1,
  KEY_HOLD  = 2
} KeyState;

void gpio_init();
uint8_t gpio_pin(uint8_t key);
KeyState gpio_update_key(uint8_t key);


#endif // __DAWN_GPIO_H__
