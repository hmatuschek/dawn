#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define UBRR_VAL  ((F_CPU+UART_BAUD*8)/(UART_BAUD*16)-1)
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))

#define min(a,b) (a>b ? b : a)

volatile static uint8_t rx_buffer[UART_BUFFER_SIZE];
volatile static uint16_t rx_buffer_idx_in = 0;
volatile static uint16_t rx_buffer_idx_out = 0;
volatile static uint8_t tx_buffer[UART_BUFFER_SIZE];
volatile static uint16_t tx_buffer_idx_in = 0;
volatile static uint16_t tx_buffer_idx_out = 0;

void
uart_init() {
  UBRR0H = UBRR_VAL >> 8;
  UBRR0L = UBRR_VAL & 0xFF;
  UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);

  rx_buffer_idx_in = rx_buffer_idx_out = 0;
  tx_buffer_idx_in = tx_buffer_idx_out = 0;
}

uint16_t
uart_write_size() {
  if (tx_buffer_idx_in == tx_buffer_idx_out) { return 0; }
  if (tx_buffer_idx_in > tx_buffer_idx_out) {
    return (tx_buffer_idx_in-tx_buffer_idx_out);
  }
  // if wrap around (in < out)
  return (UART_BUFFER_SIZE-tx_buffer_idx_out+tx_buffer_idx_in);
}

uint16_t
uart_write(const uint8_t *buffer, uint16_t n) {
  // Determine space left in buffer
  n = min(n, (UART_BUFFER_SIZE-uart_write_size()));
  for (uint16_t i=0; i<n; i++) {
    tx_buffer[tx_buffer_idx_in] = buffer[i];
    tx_buffer_idx_in = ( (tx_buffer_idx_in+1) % UART_BUFFER_SIZE);
  }
  // enable TX interrupt
  UCSR0B |= (1<<UDRIE0);
  return n;
}

void
uart_putc(uint8_t c) {
  while(UART_BUFFER_SIZE == uart_write_size()) { }
  tx_buffer[tx_buffer_idx_in] = c;
  tx_buffer_idx_in = ( (tx_buffer_idx_in+1) % UART_BUFFER_SIZE );
}

uint16_t
uart_read_size() {
  if (rx_buffer_idx_in == rx_buffer_idx_out) { return 0; }
  if (rx_buffer_idx_in > rx_buffer_idx_out) {
    return (rx_buffer_idx_in-rx_buffer_idx_out);
  }
  // if wrap around (in < out)
  return (UART_BUFFER_SIZE-rx_buffer_idx_out+rx_buffer_idx_in);
}

uint16_t
uart_read(uint8_t *buffer, uint16_t n) {
  // Determine space left in buffer
  n = min(n, (UART_BUFFER_SIZE-uart_read_size()));
  for (uint16_t i=0; i<n; i++) {
    buffer[i] = rx_buffer[rx_buffer_idx_out];
    rx_buffer_idx_out = (rx_buffer_idx_out+1) % UART_BUFFER_SIZE;
  }
  return n;
}

uint8_t
uart_peek() {
  // if empty -> return 'nul'
  if (rx_buffer_idx_out==rx_buffer_idx_in) { return 0; }
  return rx_buffer[rx_buffer_idx_out];
}

void
uart_poke() {
  // if empty -> skip
  if (rx_buffer_idx_out==rx_buffer_idx_in) { return; }
  rx_buffer_idx_out = ( (rx_buffer_idx_out+1) % UART_BUFFER_SIZE );
}

uint8_t
uart_getc() {
  while (rx_buffer_idx_out==rx_buffer_idx_in) { }
  uint8_t c = rx_buffer[rx_buffer_idx_out];
  rx_buffer_idx_out = ( (rx_buffer_idx_out+1) % UART_BUFFER_SIZE);
  return c;
}


// On char received
ISR(USART_RX_vect)
{
  // Get char
  uint8_t nextChar = UDR0;
  // Store in buffer
  rx_buffer[rx_buffer_idx_in] = nextChar;
  // Increment in buffer pointer
  rx_buffer_idx_in = ( (rx_buffer_idx_in+1) % UART_BUFFER_SIZE );
  // if overflow -> increment read pointer
  if (rx_buffer_idx_out == rx_buffer_idx_in) {
    rx_buffer_idx_out = ( (rx_buffer_idx_out+1) % UART_BUFFER_SIZE );
  }
}


// On char send
ISR(USART_UDRE_vect) {
  if (tx_buffer_idx_in == tx_buffer_idx_out) {
    // if buffer is empty -> diable TX interrupt
    UCSR0B &= ~(1<<UDRIE0);
  } else {
    // if buffer is not empty -> send a char
    uint8_t nextChar = tx_buffer[tx_buffer_idx_out];
    tx_buffer_idx_out = ( (tx_buffer_idx_out+1) % UART_BUFFER_SIZE );
    UDR0 = nextChar;
  }
}

