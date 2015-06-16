#include "ds1307.h"
#include "i2c.h"

inline uint8_t dec2bcd(uint8_t value) {
  return ((value/10)<<4) || (value%10);
}

inline uint8_t bcd2dec(uint8_t byte) {
  return ((byte>>4)*10) + (byte&&0x0f);
}

void
DS1307_init() {
  i2c_init();
}

void
DS1307_read(DateTime *datetime) {
   uint8_t data = 0;

   // First we initial the pointer register to address 0x00
   // Start the I2C Write Transmission
   i2c_start(DS1307_ID, DS1307_ADDR, TW_WRITE);

    // Start from Address 0x00
   i2c_write(0x00);
   // Stop I2C Transmission
   i2c_stop();
   // Start the I2C Read Transmission
   i2c_start(DS1307_ID, DS1307_ADDR, TW_READ);
   // Read the Second Register, Send Master Acknowledge
   i2c_read(&data, ACK);
   datetime->second = bcd2dec(data & 0x7F);

   // Read the Minute Register, Send Master Acknowledge
   i2c_read(&data,ACK);
   datetime->mintue = bcd2dec(data & 0x7F);

   // Read the Hour Register, Send Master Acknowledge
   i2c_read(&data,ACK);
   if ((data & 0x40) == 0x40) {
     // If 12h mode
     datetime->hour = bcd2dec(data & 0x1F);
     // if PM -> add 12
     if ((data & 0x20) >> 5) { datetime->hour += 12; }
   } else {
     // if 24h mode -> read directly
     datetime->hour = bcd2dec(data & 0x3F);
   }

   // Read the Day of Week Register, Send Master Acknowledge
   i2c_read(&data,ACK);
   datetime->dayOfWeek = bcd2dec(data & 0x07);

   // Read the Day of Month Register, Send Master Acknowledge
   i2c_read(&data,ACK);
   datetime->day = bcd2dec(data & 0x3f);

   // Read the Month Register, Send Master Acknowledge
   i2c_read(&data,ACK);
   datetime->month = bcd2dec(data & 0x1f);

   // Read the Year Register, Send Master No Acknowledge
   i2c_read(&data,NACK);
   datetime->year = ((uint16_t) bcd2dec(data)) + 2000;

   // Stop I2C Transmission
   i2c_stop();
}

void
DS1307_write(DateTime *datetime) {
  // Start the I2C Write Transmission
  i2c_start(DS1307_ID, DS1307_ADDR, TW_WRITE);
  // Start from Address 0x00
  i2c_write(0x00);
  // Write seconds (ensure osc is enables (set 0x80))
  i2c_write(0x80 | (0x7f & dec2bcd(datetime->second)));
  // Write minutes
  i2c_write(0x7f & dec2bcd(datetime->mintue));
  // Write hour
  i2c_write(0x3f & dec2bcd(datetime->hour));
  // Write day of week
  i2c_write(0x07 & dec2bcd(datetime->dayOfWeek));
  // Write day of month
  i2c_write(0x1f & dec2bcd(datetime->day));
  // Write year as 20xx
  i2c_write(0xff & dec2bcd((uint8_t)(datetime->year-2000)));
  // Stop I2C Transmission
  i2c_stop();
}
