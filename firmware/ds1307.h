#ifndef __LAMPE_DS1307_H__
#define __LAMPE_DS1307_H__

#include "inttypes.h"

#define DS1307_ID    0xD0        // I2C DS1307 Device Identifier
#define DS1307_ADDR  0x00        // I2C DS1307 Device Address

typedef struct {
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  dayOfWeek;
  uint8_t  hour;
  uint8_t  mintue;
  uint8_t  second;
} DateTime;

void ds1307_init();
void ds1307_read(DateTime *datetime);
void ds1307_write(DateTime *datetime);


#endif // __LAMPE_DS1307_H__
