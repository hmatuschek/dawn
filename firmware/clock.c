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


// Specifies the maximum duration of the alarm (2h)
#define CLOCK_ALARM_MAX_SEC 7200

typedef enum {
  CLOCK_WAIT,
  CLOCK_ALARM
} ClockState;

typedef struct {
  ClockState state;
  uint16_t   value;
  uint16_t   ticks;
  uint16_t   alarmSeconds;
  DateTime   datetime;
  Alarm      alarm[CLOCK_N_ALARM];
} Clock;

// The singleton clock instance
volatile static Clock clock;

// Persistent storage of alarm settings
Alarm storedAlarm[CLOCK_N_ALARM] EEMEM = {
  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };


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
  clock.alarmSeconds = 0;

  // init keys
  touch_init();

  // init pwm
  pwm_init();

  // init RTC
  ds1307_init();

  // get current time
  ds1307_getdate((uint8_t *) &clock.datetime.year, (uint8_t *) &clock.datetime.month, (uint8_t *) &clock.datetime.day,
                 (uint8_t *) &clock.datetime.hour, (uint8_t *) &clock.datetime.minute, (uint8_t *) &clock.datetime.second);
  clock.datetime.dayOfWeek =
      ds1307_getdayofweek(clock.datetime.year, clock.datetime.month, clock.datetime.day);

  // Read alarm settings from EEPROM
  eeprom_read_block((Alarm *)clock.alarm, storedAlarm, 7*sizeof(Alarm));

  // Configure timer0 to trigger interrupt about every 10ms
  TCCR0A =
      // No PWM -> CTC (reset on compare match), OC0B disconnected
      (1 << WGM01);
  TCCR0B =
      // prescaler 1024
      (1 << CS02) | (1 << CS00);
  OCR0A =
      // Output compare register A  16Mhz/1024/0x9c = 100.16 Hz
      0x9c;
  TIMSK0 =
      // Interrupt on output compare A match
      (1 << OCIE0A);
}


// Tiny helper function to check if a alarm is triggered
uint8_t alarm_match() {
  for (int i=0; i<CLOCK_N_ALARM; i++) {
    // check if day of week matches
    if (0 == (clock.alarm[i].select>>clock.datetime.dayOfWeek)) { continue; }
    if (clock.alarm[i].hour != clock.datetime.hour) { continue; }
    if (clock.alarm[i].minute != clock.datetime.minute) { continue; }
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

uint8_t clock_set_datetime(DateTime *datetime) {
  // Try to set DS1307 RTC
  if(! ds1307_setdate(datetime->year, datetime->month, datetime->day,
                      datetime->hour, datetime->minute, datetime->second)) { return 0; }
  // On success update local date-time struct
  memcpy((DateTime *) &clock.datetime, datetime, sizeof(clock.datetime));
  return 1;
}

uint8_t clock_set_alarm(uint8_t idx, Alarm *alarm) {
  if (idx >= CLOCK_N_ALARM) { return 0; }
  memcpy((Alarm *) &clock.alarm[idx], alarm, sizeof(clock.alarm));
  // store alarm config into EEPROM
  eeprom_write_block((Alarm *)clock.alarm, storedAlarm, 7*sizeof(Alarm));
  return 1;
}

void clock_get_alarm(uint8_t idx, Alarm *alarm) {
  if (idx >= CLOCK_N_ALARM) {
    alarm->select = alarm->hour = alarm->minute = 0;
    return;
  } else {
    memcpy(alarm, (Alarm *) &clock.alarm[idx], sizeof(Alarm));
  }
}


// Interrupt service routine for TIMER0
ISR(TIMER0_COMPA_vect) {
  cli();
  // increment tick counter
  clock.ticks++;

  // Every second
  if (clock.ticks == 100) {
    clock.ticks = 0;
    // Update date and time
    ds1307_getdate((uint8_t *) &(clock.datetime.year), (uint8_t *) &(clock.datetime.month), (uint8_t *) &(clock.datetime.day),
                   (uint8_t *) &(clock.datetime.hour), (uint8_t *) &(clock.datetime.minute), (uint8_t *) &(clock.datetime.second));
    clock.datetime.dayOfWeek =
        ds1307_getdayofweek(clock.datetime.year, clock.datetime.month, clock.datetime.day);
    // If alarm -> update alarm seconds counter
    if (CLOCK_ALARM == clock.state) {
      clock.alarmSeconds++;
      if (CLOCK_ALARM_MAX_SEC <= clock.alarmSeconds) {
        clock.state = CLOCK_WAIT;
        clock.value = 0;
        pwm_set(clock.value);
      }
    }
    // Check for alarm if enabled (pin0 -> high)
    if ( (CLOCK_WAIT == clock.state) && (0x00ff > clock.value) && alarm_match() ) {
      // Start alarm
      clock.state = CLOCK_ALARM;
      clock.alarmSeconds = 0;
    }
  }

  // Every 50ms -> update dawn table index
  if ((clock.ticks % 5) && (CLOCK_ALARM == clock.state)) {
    if (0xFFFF > clock.value) { clock.value++; }
    pwm_set(clock_map_dawn_func(clock.value));
  }

  // Check down key:
  touch_update_keys();

  /*switch (gpio_update_key(0)) {
  case KEY_KLICK:
    if (CLOCK_ALARM == clock.state) {
      // Interrupt alarm
      clock.state = CLOCK_WAIT;
    }
    // Switch off
    clock.value = 0; pwm_set(clock.value);
    break;
  case KEY_HOLD:
    if (CLOCK_ALARM == clock.state) {
      // Interrupt alarm
      clock.state = CLOCK_WAIT;
    }
    // Decrease current value
    if (clock.value) {
      clock.value--;
    }
    pwm_set(clock.value);
    break;
  case KEY_NONE:
    break;
  }

  // Check up key
  switch (gpio_update_key(1)) {
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
  } */

  sei();
  return;
}

