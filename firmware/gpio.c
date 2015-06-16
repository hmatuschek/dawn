#include "gpio.h"
#include <avr/io.h>

// PIN0 -> PB0
// PIN1 -> PD7
// PIN2 -> PD6
// PIN3 -> PD5

#define KEY_HOLD_DUR  100

typedef struct {
  KeyState last_state;
  uint16_t tick_count;
} KeyContext;

static KeyContext keys[4] = {
  {KEY_NONE, 0},
  {KEY_NONE, 0},
  {KEY_NONE, 0},
  {KEY_NONE, 0} };

void
gpio_init() {
  // PB1, PD7, PD6, PD5 is now an output
  DDRB &= ~(1 << DDB0);
  DDRD &= ~((1 << DDB7) | (1 << DDB6) | (1 << DDB5));
  // Enables pull-ups
  PORTB |= (1 << DDB0);
  PORTD |= (1 << DDB7) | (1 << DDB6) | (1 << DDB5);
  // Init keys
  keys[0].last_state = KEY_NONE;  keys[0].tick_count = 0;
  keys[1].last_state = KEY_NONE;  keys[1].tick_count = 0;
  keys[2].last_state = KEY_NONE;  keys[2].tick_count = 0;
  keys[3].last_state = KEY_NONE;  keys[3].tick_count = 0;
}


uint8_t
gpio_pin(uint8_t key) {
  switch (key) {
  case 0: return 0 == (PINB & (1<<DDB0));
  case 1: return 0 == (PIND & (1<<DDB7));
  case 2: return 0 == (PIND & (1<<DDB6));
  case 3: return 0 == (PIND & (1<<DDB5));
  default: break;
  }
  return 0;
}

KeyState
gpio_update_key(uint8_t key)
{
  // There are only 4 keys
  if (key > 3) { return KEY_NONE; }
  // On key down
  if (gpio_pin(key)) {
    // If key pressed
    if (KEY_NONE == keys[key].last_state) {
      keys[key].last_state = KEY_KLICK;
      keys[key].tick_count = 1;
      return KEY_NONE;
    }
    // increment tick counter
    keys[key].tick_count++;
    // If key was pressed longer than KEY_HOLD_DUR
    if (keys[key].tick_count >= KEY_HOLD_DUR) {
      keys[key].tick_count = 1;
      keys[key].last_state = KEY_HOLD;
      return KEY_HOLD;
    } else if (KEY_HOLD == keys[key].last_state) {
      return KEY_HOLD;
    }
    return KEY_NONE;
  }
  // On key up
  if (KEY_NONE == keys[key].last_state) { return KEY_NONE; }
  if (KEY_KLICK == keys[key].last_state) {
    keys[key].last_state = KEY_NONE;
    return KEY_KLICK;
  }
  keys[key].last_state = KEY_NONE;
  return KEY_NONE;
}
