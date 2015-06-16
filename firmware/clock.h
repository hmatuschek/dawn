#ifndef __LAMPE_CLOCK_H__
#define __LAMPE_CLOCK_H__

#include "ds1307.h"

#ifndef N_ALARM
#define CLOCK_N_ALARM 7
#endif

typedef struct {
  uint8_t select;
  uint8_t hour;
  uint8_t minute;
} Alarm;

/** Initializes the clock (alarm, pwm, RTC, keys etc.). */
void clock_init();

void clock_set_datetime(DateTime *datetime);
void clock_get_datetime(DateTime *datetime);

void clock_get_alarm(uint8_t idx, Alarm *alarm);
void clock_set_alarm(uint8_t idx, Alarm *alarm);

uint16_t clock_get_value();
void clock_set_value(uint16_t value);

#endif // __LAMPE_CLOCK_H__
