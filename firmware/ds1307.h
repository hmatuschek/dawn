/*
 * I2C interface to the DS1307 real-time clock (RTC) chip.
 */
#ifndef __DAWN_DS1307_H__
#define __DAWN_DS1307_H__

#include "inttypes.h"

#define DS1307_ID    0xD0        // I2C DS1307 Device Identifier
#define DS1307_ADDR  0x00        // I2C DS1307 Device Address

/** Represents a date and time, including day of week. */
typedef struct {
  uint16_t year;      ///< The year in [2000,2099].
  uint8_t  month;     ///< The month in [1,21].
  uint8_t  day;       ///< The day in [1,31].
  uint8_t  dayOfWeek; ///< The day of week [0,6] where 0=sun, ..., 6=sat.
  uint8_t  hour;      ///< The hour in [0,23].
  uint8_t  mintue;    ///< The minute in [0,59].
  uint8_t  second;    ///< The second in [0,59].
} DateTime;

/** Initializes the I2C interface. */
void ds1307_init();
/** Reads the current date & time from the RTC. */
void ds1307_read(DateTime *datetime);
/** Writes the specified date & time to the RTC. */
void ds1307_write(DateTime *datetime);

#endif // __DAWN_DS1307_H__
