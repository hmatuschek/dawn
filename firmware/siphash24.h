#ifndef __DAWN_SIPHASH_H__
#define __DAWN_SIPHASH_H__

/*
 * siphash.h
 * SipHash for 8bit Atmel processors
 *
 * Note: one instance sipHash is already constructed in .cpp file
 *
 * Usage
 * uint8_t key[] PROGMEM = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 *                          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
 * // initialize with key NOTE: key is stored in flash (PROGMEM)
 * siphash_init_from_progmem(key);
 * // use siphash_init(key); if key in RAM
 * for (int i=0; i<msgLen;i++) {
 *  // update hash with each byte of msg
 *  siphash_update((uint8_t)c);
 * }
 * siphash_finish(); // finish
 * siphash_result then contains the 8bytes of the hash in BigEndian format
 *
 * see https://131002.net/siphash/ for details of algorithm
 *
 * (c)2013 Forward Computing and Control Pty. Ltd.
 * www.forward.com.au
 * This code may be freely used for both private and commercial use.
 * Provide this copyright is maintained.
 */

#include <inttypes.h>
#include <avr/pgmspace.h>

extern void reverse64(uint8_t *x);

extern void siphash_cbc_mac_progmem(uint8_t *hash, const uint8_t *data, uint8_t len, const uint8_t *secret);
extern void siphash_cbc_mac(uint8_t *hash, const uint8_t *data, uint8_t len, const uint8_t *secret);

extern void siphash_init_from_progmem(const uint8_t *keyPrgPtrIn);
extern void siphash_init(const uint8_t *key);
extern void siphash_update(uint8_t c);
extern void siphash_finish();
extern void siphash_get(uint8_t *hash);

#endif // __DAWN_SIPHASH_H__
