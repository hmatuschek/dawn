#ifndef __DAWN_CLOCK_H__
#define __DAWN_CLOCK_H__

#include "ds1307.h"

#ifndef CLOCK_N_ALARM
#define CLOCK_N_ALARM 7
#endif

typedef struct {
  uint8_t select;
  uint8_t hour;
  uint8_t minute;
} Alarm;

/** Initializes the clock (alarm, pwm, RTC, keys etc.). */
void clock_init();

/** Sets the current date and time. */
void clock_set_datetime(DateTime *datetime);
/** Reads the current date and time. */
void clock_get_datetime(DateTime *datetime);

/** Gets the specified alarm settings. */
void clock_get_alarm(uint8_t idx, Alarm *alarm);
/** Updates the specified alarm settings. */
void clock_set_alarm(uint8_t idx, Alarm *alarm);

/** Returns the current luminescence value. */
uint16_t clock_get_value();
/** Sets the current luminescence value. */
void clock_set_value(uint16_t value);

#endif // __DAWN_CLOCK_H__
