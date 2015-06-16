#include "clock.h"
#include "ds1307.h"
#include "pwm.h"
#include "gpio.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <string.h>
#include "dawnfunction.h"

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

// The singleton clock instance
volatile static Clock clock;

// Persistent storage of alarm settings
Alarm storedAlarm[7] EEMEM;


// interpolation function for the dawn function table
uint16_t clock_map_dawn_func(uint16_t value) {
  uint8_t idx  = (value>>8);
  uint8_t frac = (value & 0xff);
  uint32_t a = (0 == idx) ? 0 : dawn_func[idx-1];
  uint32_t b = dawn_func[idx];
  uint16_t x = ((0x100-frac)*a + frac*b)/512;
  return x;
}


void
clock_init() {
  // Initialize clock state
  clock.state = CLOCK_WAIT;
  clock.value = 0;
  clock.ticks = 0;

  // init keys
  gpio_init();

  // init pwm
  pwm_init();

  // init RTC
  ds1307_init();
  // get current time
  ds1307_read((DateTime *) &clock.datetime);

  // Read alarm settings from EEPROM
  eeprom_read_block(clock.alarm, storedAlarm, 7*sizeof(Alarm));

  // Configure timer0 to trigger interrupt every 10ms
  TCCR0 = (1<<CS02)|(1<<CS00);    // Use maximum prescaller: Clk/1024
  TCNT0  = 0x94;                  // Start counter from 0x94, overflow at 10 mSec
  TIMSK = (1<<TOIE0);             // Enable Counter Overflow Interrupt
}


// Tiny helper function to check if a alarm is triggered
uint8_t alarm_match() {
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
  // store alarm config into EEPROM
  eeprom_write_block(clock.alarm, storedAlarm, 7*sizeof(Alarm));
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
    // Update date and time
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

  sei();
  TCNT0 = 0x94; // enable timer0 interrupt
}

