#ifndef __DAWN_I2C_H__
#define __DAWN_I2C_H__

#include <util/twi.h>

typedef enum {
  I2C_START = 0,
  I2C_DATA = 1,
  I2C_DATA_ACK = 2,
  I2C_STOP = 3
} I2CMsgType;

#define ACK 1
#define NACK 0

void i2c_init();
char i2c_start(unsigned int dev_id, unsigned int dev_addr, unsigned char rw_type);
void i2c_stop(void);
char i2c_write(char data);
char i2c_read(uint8_t *data, char ack_type);


#endif // __DAWN_I2C_H__
