#include "gpio.h"
#include <avr/io.h>

// PIN0 -> PB0
// PIN1 -> PD7
// PIN2 -> PD6
// PIN3 -> PD5

#define KEY_HOLD_DUR  100

typedef struct {
  KeyState last_state;
  KeyState state;
  uint16_t tick_count;
} KeyContext;

static KeyContext keys[3] = {
  {KEY_NONE, KEY_NONE, 0},
  {KEY_NONE, KEY_NONE, 0},
  {KEY_NONE, KEY_NONE, 0}
};

void
key_init() {
  // PD7, PD6, PD5 are inputs
  DDRD  &= ~((1 << DDD7) | (1 << DDD6) | (1 << DDD5));
  // Enables pull-ups
  PORTD |= ((1 << DDD7) | (1 << DDD6) | (1 << DDD5));
  // Init keys
  keys[0].last_state = keys[0].state = KEY_NONE;  keys[0].tick_count = 0;
  keys[1].last_state = keys[1].state = KEY_NONE;  keys[1].tick_count = 0;
  keys[2].last_state = keys[2].state = KEY_NONE;  keys[2].tick_count = 0;
}


void key_update(uint8_t key) {
  // Get current output values:
  uint8_t k   = 1;
  switch (key) {
  case 0: k = (PIND & (1<<DDD7)); break;
  case 1: k = (PIND & (1<<DDD6)); break;
  case 2: k = (PIND & (1<<DDD5)); break;
  }
  if (0 == k) {
    switch (keys[key].last_state) {
    case KEY_NONE:
      keys[key].last_state = KEY_CLICK;
      keys[key].tick_count = 0;
      break;
    case KEY_CLICK:
      if (keys[key].tick_count == 5) {
        keys[key].last_state = KEY_HOLD;
        keys[key].state      = KEY_HOLD;
      } else {
        keys[key].tick_count++;
      }
      break;
    case KEY_HOLD:
      break;
    }
  } else {
    switch (keys[key].last_state) {
    case KEY_NONE:
      keys[key].state = KEY_NONE;
      break;
    case KEY_CLICK:
      keys[key].last_state = KEY_NONE;
      keys[key].state      = KEY_CLICK;
      break;
    case KEY_HOLD:
      keys[key].last_state = KEY_NONE;
      keys[key].state      = KEY_NONE;
      break;
    }
    keys[key].tick_count = 0;
  }
}


KeyState key(uint8_t key) {
  return keys[key].state;
}
