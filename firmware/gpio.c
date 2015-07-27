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
  {KEY_NONE, 0},
  {KEY_NONE, 0},
  {KEY_NONE, 0}};

void
touch_init() {
  // PB0 is output
  DDRB  |= (1 << DDB0);
  // Set zero
  PORTB &= ~(1 << DDB0);
  // PD7, PD6, PD5 are inputs
  DDRD  &= ~((1 << DDD7) | (1 << DDD6) | (1 << DDD5));
  // Disable pull-ups
  PORTD &= ~((1 << DDD7) | (1 << DDD6) | (1 << DDD5));
  // Init keys
  keys[0].last_state = KEY_NONE;  keys[0].tick_count = 0;
  keys[1].last_state = KEY_NONE;  keys[1].tick_count = 0;
  keys[2].last_state = KEY_NONE;  keys[2].tick_count = 0;
}


KeyState
touch_pin(uint8_t key) {
  // There are only 3 keys
  if (key > 2) { return KEY_NONE; }
  if (KEY_KLICK == keys[key].state) {
    keys[key].state = KEY_NONE;
    return KEY_KLICK;
  }
  return keys[key].state;
}

void gpio_update_key(uint8_t key) {
  // Get current output values:
  uint8_t k   = 0;
  switch (key) {
  case 0: k = (PORTD & (1<<DDD7)); break;
  case 1: k = (PORTD & (1<<DDD6)); break;
  case 2: k = (PORTD & (1<<DDD5)); break;
  }
  if (k) {
    switch (keys[key].last_state) {
    case KEY_NONE:
      keys[key].last_state = KEY_KLICK;
      keys[key].tick_count = 0;
      break;
    case KEY_KLICK:
      keys[key].tick_count++;
      if (keys[key].tick_count > 500) {
        keys[key].last_state = KEY_HOLD;
        keys[key].state      = KEY_HOLD;
      }
      break;
    case KEY_HOLD:
      break;
    }
  } else {
    switch (keys[key].last_state) {
    case KEY_NONE:
      break;
    case KEY_KLICK:
      keys[key].last_state = KEY_NONE;
      keys[key].state      = KEY_KLICK;
      break;
    case KEY_HOLD:
      keys[key].last_state = KEY_NONE;
      keys[key].state      = KEY_NONE;
      break;
    }
  }
}

void touch_update_keys()
{
  // Check state only if charging C
  if (PORTB & (1<<DDB1)) {
    // update current key states
    gpio_update_key(0);
    gpio_update_key(1);
    gpio_update_key(2);
  }
  // toggle supply pin
  PORTB ^= (1<<DDB1);
}
