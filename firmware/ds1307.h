/*
ds1307 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References: parts of the code taken from https://github.com/adafruit/RTClib
*/


#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

//definitions
#define DS1307_ADDR 0xd0 //device address

//functions
extern void ds1307_init();
extern uint8_t ds1307_setdate(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
extern void ds1307_getdate(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second);
extern uint8_t ds1307_getdayofweek(uint8_t y, uint8_t m, uint8_t d);
extern uint8_t ds1307_bcd2dec(uint8_t val);
#endif

