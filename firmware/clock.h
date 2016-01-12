#ifndef __DAWN_CLOCK_H__
#define __DAWN_CLOCK_H__

#include "ds1307.h"

#ifndef CLOCK_N_ALARM
#define CLOCK_N_ALARM 7
#endif

/** Alarm selection. */
typedef struct {
  uint8_t select;  ///< Specifies the days of the week at which the alarm is active.
                   ///< Sat: b6, Fri: b5, Thr: b4, Wen: b3, Tue: b2, Mon: b1, Sun: b0
                   ///< => 0x00: None, 0x7f: All days
  uint8_t hour;    ///< Specifies the hour of the alarm.
  uint8_t minute;  ///< Specifies the minute of the alarm.
} Alarm;

/** Date-time structure. */
typedef struct {
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t dayOfWeek;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} DateTime;

/** Initializes the clock (alarm, pwm, RTC, keys etc.). */
void clock_init();

/** Sets the current date and time. */
uint8_t clock_set_datetime(DateTime *datetime);
/** Reads the current date and time. */
void clock_get_datetime(DateTime *datetime);

/** Gets the specified alarm settings. */
void clock_get_alarm(uint8_t idx, Alarm *alarm);
/** Updates the specified alarm settings. */
uint8_t clock_set_alarm(uint8_t idx, Alarm *alarm);

/** Returns the current luminescence value. */
uint16_t clock_get_value();
/** Sets the current luminescence value. */
void clock_set_value(uint16_t value);

#endif // __DAWN_CLOCK_H__
