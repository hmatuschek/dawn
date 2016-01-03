#ifndef __DAWN_GPIO_H__
#define __DAWN_GPIO_H__

#include <inttypes.h>

typedef enum {
  KEY_NONE  = 0,
  KEY_CLICK = 1,
  KEY_HOLD  = 2
} KeyState;

void key_init();
void key_update(uint8_t key);
void key_update_keys();
KeyState key(uint8_t key);

#endif // __DAWN_GPIO_H__
