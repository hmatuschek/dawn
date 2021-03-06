#include "communication.h"
#include "uart.h"
#include "siphash24.h"
#include <string.h>

// Needs to be included before secret.
#include <avr/pgmspace.h>
#include "secret.h"

static uint8_t nonce[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void
comm_init() {
  uart_init(UART_BAUD_9600);
}

// Internal used function to read complete message once the
// command ID is known.
uint8_t __comm_get(Command *cmd) {
  uint8_t size = 0;
  // Dispatch payload size by cmd type
  switch (cmd->command) {
    case GET_VALUE:  size = 0; break;
    case SET_VALUE:  size = 2; break;
    case GET_TIME:   size = 0; break;
    case SET_TIME:   size = 7; break;
    case GET_ALARM:  size = 1; break;
    case SET_ALARM:  size = 4; break;
    case GET_TEMP:   size = 0; break;
    case GET_NALARM: size = 0; break;
    case GET_NONCE:  size = 0; break;
    default: return 0;
  }
  // Wait for the payload
  while (size > uart_available()) { }
  uart_read((uint8_t *) &(cmd->payload), size);
  // Wait for the MAC
  while (8 > uart_available()) { }
  uart_read(cmd->mac, 8);

  // Check MAC
  uint8_t hash[8];
  siphash_cbc_mac_progmem(hash, (uint8_t *) cmd, size+1, nonce, secret);
  // Ignore hash if CMD == GET_NONCE
  if (GET_NONCE != cmd->command) {
    for (uint8_t i=0; i<8; i++) {
      if (hash[i] != cmd->mac[i]) { return 0; }
    }
    // On success update nonce
    siphach_cbc_update_nonce_progmem(nonce, hash, secret);
  }
  return 1;
}


uint8_t
comm_wait(Command *cmd) {
  // Wait for command char
  while (0 == uart_available()) ;
  cmd->command = uart_getc();
  // Try to read complete command:
  if (__comm_get(cmd)) {
    return 1;
  }
  return 0;
}

void
comm_send_ok() {
  uart_putc(0x00);
}

void
comm_send_err() {
  uart_putc(0xff);
}

void
comm_send_value(uint16_t value) {
  uart_putc(0x00);
  uart_putc(value>>8);
  uart_putc(value);
}

void
comm_send_time(DateTime *datetime) {
  uart_putc(0x00);
  uart_putc(datetime->year);
  uart_putc(datetime->month);
  uart_putc(datetime->day);
  uart_putc(datetime->dayOfWeek);
  uart_putc(datetime->hour);
  uart_putc(datetime->minute);
  uart_putc(datetime->second);
}

void
comm_send_alarm(Alarm *alarm) {
  uart_putc(0x00);
  uart_putc(alarm->select);
  uart_putc(alarm->hour);
  uart_putc(alarm->minute);
}

void
comm_send_temp(uint16_t core, uint16_t ambient) {
  uart_putc(0x00);
  uart_putc(core>>8);    uart_putc(core);
  uart_putc(ambient>>8); uart_putc(ambient);
}

void
comm_send_nalarm(uint8_t nalarm) {
  uart_putc(0x00);
  uart_putc(nalarm);
}

void
comm_send_nonce() {
  uart_putc(0x00);
  uart_write(nonce, 8);
}
