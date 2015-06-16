#include "communication.h"
#include "uart.h"
#include "siphash24.h"
#include "secret.h"

void
comm_init() {
  uart_init();
}


void
comm_wait(Command *cmd) {
  while (1) {
    cmd->command = uart_getc();
    if ((cmd->command>CMD_MIN) && (cmd->command<CMD_MAX)) {
      if (comm_get(cmd)) { return; }
    }
  }
}


uint8_t
comm_get(Command *cmd) {
  uint8_t hash[8] = {0,0,0,0,0,0,0,0};

  uint16_t size = 0;
  // Dispatch command size by cmd type
  switch (cmd->command) {
  case GET_VALUE: size = 9; break;
  case SET_VALUE: size = 11; break;
  case GET_TIME:  size = 9; break;
  case SET_TIME:  size = 17; break;
  case GET_ALARM: size = 10; break;
  case SET_ALARM: size = 13; break;
  }
  // Wait for the complete packet
  while (size > uart_read_size()) { }
  uint8_t *ptr = (uint8_t *)cmd; ptr++;
  uart_read(ptr, size-1);

  // Check MAC
  ptr = (uint8_t *)cmd;
  siphash24_cbc_mac(hash, ptr, size-8, secret);
  uint8_t correct = 1;
  for (uint16_t i=0; i<(size-8); i++) {
    correct = ( correct && (hash[i]==cmd->mac[i]) );
  }
  return correct;
}

void
comm_send_ok() {
  uart_putc(0x00);
}

void
comm_send_value(uint16_t value) {
  uart_putc(value>>8);
  uart_putc(value&0xff);
}

void
comm_send_time(DateTime *datetime) {
  uart_putc(datetime->year >> 8);
  uart_putc(datetime->year & 0xff);
  uart_putc(datetime->month);
  uart_putc(datetime->day);
  uart_putc(datetime->dayOfWeek);
  uart_putc(datetime->hour);
  uart_putc(datetime->mintue);
  uart_putc(datetime->second);
}

void
comm_send_alarm(Alarm *alarm) {
  uart_putc(alarm->select);
  uart_putc(alarm->hour);
  uart_putc(alarm->minute);
}

