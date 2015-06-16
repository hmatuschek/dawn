#include "clock.h"
#include "ds1307.h"
#include "pwm.h"
#include "gpio.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

typedef enum {
  CLOCK_WAIT,
  CLOCK_ALARM
} ClockState;

typedef struct {
  ClockState state;
  uint16_t   value;
  uint16_t   increment;
  uint16_t   ticks;
  DateTime   datetime;
  Alarm      alarm[CLOCK_N_ALARM];
} Clock;

volatile static Clock clock;

const uint16_t dawn_func[] PROGMEM = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002,
  0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0004, 0x0004, 0x0004, 0x0004, 0x0005,
  0x0005, 0x0005, 0x0005, 0x0006, 0x0006, 0x0006, 0x0007, 0x0007,
  0x0007, 0x0008, 0x0008, 0x0009, 0x0009, 0x000a, 0x000a, 0x000b,
  0x000b, 0x000c, 0x000c, 0x000d, 0x000e, 0x000e, 0x000f, 0x0010,
  0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018,
  0x0019, 0x001b, 0x001c, 0x001d, 0x001f, 0x0020, 0x0022, 0x0024,
  0x0026, 0x0028, 0x002a, 0x002c, 0x002e, 0x0030, 0x0033, 0x0035,
  0x0038, 0x003b, 0x003e, 0x0041, 0x0044, 0x0048, 0x004c, 0x004f,
  0x0054, 0x0058, 0x005c, 0x0061, 0x0066, 0x006b, 0x0070, 0x0076,
  0x007c, 0x0082, 0x0089, 0x0090, 0x0097, 0x009f, 0x00a7, 0x00b0,
  0x00b8, 0x00c2, 0x00cc, 0x00d6, 0x00e1, 0x00ec, 0x00f8, 0x0105,
  0x0112, 0x0120, 0x012e, 0x013e, 0x014e, 0x015f, 0x0171, 0x0183,
  0x0197, 0x01ab, 0x01c1, 0x01d8, 0x01f0, 0x0209, 0x0223, 0x023f,
  0x025c, 0x027a, 0x029b, 0x02bc, 0x02e0, 0x0305, 0x032c, 0x0355,
  0x0380, 0x03ae, 0x03dd, 0x040f, 0x0444, 0x047b, 0x04b5, 0x04f2,
  0x0532, 0x0575, 0x05bc, 0x0606, 0x0654, 0x06a6, 0x06fd, 0x0757,
  0x07b6, 0x081a, 0x0883, 0x08f1, 0x0965, 0x09de, 0x0a5e, 0x0ae4,
  0x0b71, 0x0c05, 0x0ca1, 0x0d45, 0x0df0, 0x0ea5, 0x0f62, 0x102a,
  0x10fb, 0x11d7, 0x12be, 0x13b0, 0x14af, 0x15bb, 0x16d4, 0x17fc,
  0x1932, 0x1a78, 0x1bcf, 0x1d37, 0x1eb1, 0x203f, 0x21e0, 0x2396,
  0x2563, 0x2747, 0x2944, 0x2b5a, 0x2d8b, 0x2fd8, 0x3244, 0x34cf,
  0x377a, 0x3a48, 0x3d3b, 0x4053, 0x4394, 0x46ff, 0x4a96, 0x4e5b,
  0x5252, 0x567b, 0x5adb, 0x5f73, 0x6446, 0x6958, 0x6eac, 0x7445,
  0x7a26, 0x8053, 0x86d0, 0x8da1, 0x94cb, 0x9c51, 0xa438, 0xac86,
  0xb53f, 0xbe69, 0xc80a, 0xd227, 0xdcc8, 0xe7f2, 0xf3ac, 0xffff };


uint16_t
clock_map_dawn_func(uint16_t value) {
  uint8_t idx  = (value>>8);
  //uint8_t frac = (value & 0xff);
  return dawn_func[idx];
}


void
clock_init() {
  clock.state = CLOCK_WAIT;
  clock.value = 0;
  clock.ticks = 0;

  gpio_init();
  pwm_init();

  ds1307_init();
  ds1307_read((DateTime *) &clock.datetime);

  TCCR0 = (1<<CS02)|(1<<CS00);    // Use maximum prescaller: Clk/1024
  TCNT0  = 0x94;                  // Start counter from 0x94, overflow at 10 mSec
  TIMSK = (1<<TOIE0);             // Enable Counter Overflow Interrupt
}


uint8_t alarm_match()
{
  for (int i=0; i<CLOCK_N_ALARM; i++) {
    // check if day of week matches
    if (0 == (clock.alarm[i].select>>clock.datetime.dayOfWeek)) { continue; }
    if (clock.alarm[i].hour != clock.datetime.hour) { continue; }
    if (clock.alarm[i].minute != clock.datetime.mintue) { continue; }
    if (1 < clock.datetime.second) { continue; }
    return 1;
  }
  return 0;
}


uint16_t clock_get_value() {
  return clock.value;
}

void clock_set_value(uint16_t value) {
  clock.value = value;
  pwm_set(clock.value);
}

void clock_get_datetime(DateTime *datetime) {
  memcpy(datetime, (DateTime *) &clock.datetime, sizeof(DateTime));
}

void clock_set_datetime(DateTime *datetime) {
  memcpy((DateTime *) &clock.datetime, datetime, sizeof(clock.datetime));
  ds1307_write((DateTime *) &clock.datetime);
}

void clock_set_alarm(uint8_t idx, Alarm *alarm) {
  if (idx >= CLOCK_N_ALARM) { return; }
  memcpy((Alarm *) &clock.alarm[idx], alarm, sizeof(clock.alarm));
}

void clock_get_alarm(uint8_t idx, Alarm *alarm) {
  if (idx >= CLOCK_N_ALARM) {
    alarm->select = alarm->hour = alarm->minute = 0;
  }
  memcpy(alarm, (Alarm *) &clock.alarm[idx], sizeof(Alarm));
}


// Interrupt service routine for TIMER0
ISR(TIMER0_OVF_vect) {
  cli(); // Disable Interupt

  // increment tick counter
  clock.ticks++;
  // Every second
  if (clock.ticks == 100) {
    clock.ticks = 0;
    ds1307_read((DateTime *) &clock.datetime);
    // Check for alarm if enabled (pin0 -> high)
    if ( gpio_pin(0) && (0x00ff > clock.value) && alarm_match() ) {
      clock.state = CLOCK_ALARM;
    }
  }

  // Update pwm value
  if ((clock.ticks % 5) && (CLOCK_ALARM == clock.state)) {
    if (0xFFFF > clock.value) { clock.value++; }
    pwm_set(clock_map_dawn_func(clock.value));
  }

  // Check down key:
  switch (gpio_update_key(1)) {
  case KEY_KLICK:
    // Interrupt alarm
    if (CLOCK_ALARM == clock.state) { clock.state = CLOCK_WAIT; }
    // Switch off
    clock.value = 0; pwm_set(clock.value);
    break;
  case KEY_HOLD:
    // Interrupt alarm
    if (CLOCK_ALARM == clock.state) { clock.state = CLOCK_WAIT; }
    // Decrease current value
    if (clock.value) { clock.value--; }
    pwm_set(clock.value);
    break;
  case KEY_NONE:
    break;
  }

  // Check up key
  switch (gpio_update_key(2)) {
  case KEY_KLICK:
    // Interrupt alarm
    if (CLOCK_ALARM == clock.state) { clock.state = CLOCK_WAIT; }
    // Switch on
    clock.value = 0xffff; pwm_set(clock.value);
    break;
  case KEY_HOLD:
    // Interrupt alarm
    if (CLOCK_ALARM == clock.state) { clock.state = CLOCK_WAIT; }
    // increment current value
    if (0xffff > clock.value) { clock.value++; }
    pwm_set(clock.value);
    break;
  case KEY_NONE:
    break;
  }

  sei(); TCNT0=0x94; // Enable Interrupt
}

