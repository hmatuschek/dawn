/*
ds1307 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include "avr/io.h"
#include "avr/pgmspace.h"
#include "util/delay.h"

#include "ds1307.h"
#include "i2c.h"

/* days per month */
const uint8_t ds1307_daysinmonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/* initialize the I2C interface */
void ds1307_init() {
	//init i2c
	i2c_init();
  _delay_ms(10);
}

/* transform decimal value to bcd */
uint8_t ds1307_dec2bcd(uint8_t val) {
	return val + 6 * (val / 10);
}

/* transform bcd value to deciaml */
uint8_t ds1307_bcd2dec(uint8_t val) {
	return val - 6 * (val >> 4);
}

/* get number of days since 2000/01/01 (valid for 2001..2099) */
static uint16_t ds1307_date2days(uint8_t y, uint8_t m, uint8_t d) {
	uint16_t days = d;
	for (uint8_t i = 1; i < m; ++i)
		days += pgm_read_byte(ds1307_daysinmonth + i - 1);
	if (m > 2 && y % 4 == 0)
		++days;
	return days + 365 * y + (y + 3) / 4 - 1;
}

/* get day of a week */
uint8_t ds1307_getdayofweek(uint8_t y, uint8_t m, uint8_t d) {
	uint16_t day = ds1307_date2days(y, m, d);
	return (day + 6) % 7;
}

/* set date */
uint8_t
ds1307_setdate(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
  // sanitize data
  if ( (second > 59) || (minute > 59) || (hour > 23) || (day < 1) ||
       (day > 31) || (month < 1) || (month > 12) || (year > 99) ) {
    return 0;
  }

  // sanitize day based on month
  if(day > pgm_read_byte(ds1307_daysinmonth + month - 1)) {
		return 0;
  }

  // get day of week
	uint8_t dayofweek = ds1307_getdayofweek(year, month, day);

  // try to get a lock for the I2C bus
  if (! i2c_lock()) { return 0; }

  // write date
  if (! i2c_start(DS1307_ADDR | I2C_WRITE))   { return 0; }
  if (! i2c_write(0x00))                      { return 0; }
  if (! i2c_write(ds1307_dec2bcd(second)))    { return 0; }
  if (! i2c_write(ds1307_dec2bcd(minute)))    { return 0; }
  if (! i2c_write(ds1307_dec2bcd(hour)))      { return 0; }
  if (! i2c_write(ds1307_dec2bcd(dayofweek))) { return 0; }
  if (! i2c_write(ds1307_dec2bcd(day)))       { return 0; }
  if (! i2c_write(ds1307_dec2bcd(month)))     { return 0; }
  if (! i2c_write(ds1307_dec2bcd(year)))      { return 0; }
  i2c_stop();

  // release I2C bus
  i2c_unlock();

	return 1;
}

/* get date */
uint8_t
ds1307_getdate(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
  // Try to lock I2C bus.
  if (! i2c_lock()) { return 0; }
  // go
  i2c_start(DS1307_ADDR | I2C_WRITE);
  i2c_write(0x00); //stop oscillator
  i2c_start(DS1307_ADDR | I2C_READ);
	*second = ds1307_bcd2dec(i2c_readAck() & 0x7F);
	*minute = ds1307_bcd2dec(i2c_readAck());
	*hour = ds1307_bcd2dec(i2c_readAck());
  i2c_readAck();
	*day = ds1307_bcd2dec(i2c_readAck());
	*month = ds1307_bcd2dec(i2c_readAck());
	*year = ds1307_bcd2dec(i2c_readNak());
	i2c_stop();
  // release bus
  i2c_unlock();
  // done.
  return 1;
}
