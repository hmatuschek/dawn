#ifndef __DAWN_UART_H__
#define __DAWN_UART_H__

#include <inttypes.h>

#ifndef UART_BAUD
#define UART_BAUD 9600L
#endif

#ifndef UART_BUFFER_SIZE
#define UART_BUFFER_SIZE 32
#endif

/** Initializes the UART for the baud rate specified by the macro
 * UART_BAUD, default 9600. */
void uart_init();

/** Returns the number of bytes in the buffer. */
uint16_t uart_read_size();
/** Reads some bytes from the buffer. */
uint16_t uart_read(uint8_t *buffer, uint16_t n);
/** Peeks at the first byte in the buffer. */
uint8_t uart_peek();
/** Drops the first byte in the buffer. */
void uart_poke();
/** Reads a single char from buffer. */
uint8_t uart_getc();

/** Returns the number of bytes in the buffer. */
uint16_t uart_write_size();
/** Writes some bytes in the buffer. */
uint16_t uart_write(const uint8_t *buffer, uint16_t n);
void uart_putc(uint8_t c);

#endif // __DAWN_UART_H__
