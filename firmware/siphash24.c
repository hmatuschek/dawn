/*
   SipHash reference C implementation

   Written in 2012 by 
   Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
   Daniel J. Bernstein <djb@cr.yp.to>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with
   this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
#include <stdint.h>
#include <string.h>
#include "siphash24.h"
#include "siphash_2_4_asm.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

unsigned const char v0_init[] PROGMEM = {0x73, 0x6f, 0x6d, 0x65, 0x70, 0x73, 0x65, 0x75};
unsigned const char v1_init[] PROGMEM = {0x64, 0x6f, 0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d};
unsigned const char v2_init[] PROGMEM = {0x6c, 0x79, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x61};
unsigned const char v3_init[] PROGMEM = {0x74, 0x65, 0x64, 0x62, 0x79, 0x74, 0x65, 0x73};

unsigned char v0[8];
unsigned char v1[8];
unsigned char v2[8];
unsigned char v3[8];

void siphash_round() {
  add64le(v0,v1);
  add64le(v2,v3);
  rol_13bits(v1);
  rol_16bits(v3);
  xor64(v1, v0);
  xor64(v3, v2);
  rol_32bits(v0);
  add64le(v2, v1);
  add64le(v0, v3);
  rol_17bits(v1);
  rol_21bits(v3);
  xor64(v1, v2);
  xor64(v3, v0);
  rol_32bits(v2);
}

/* SipHash-2-4 */
void siphash24_hash_block(unsigned char *hash, const unsigned char *block, const unsigned char *key)
{ 
  unsigned char m[8];
  
  // Init...
  memcpy_P(v0,v0_init,8);
  memcpy_P(v1,v1_init,8);
  memcpy_P(v2,v2_init,8);
  memcpy_P(v3,v3_init,8);

  // now load first 8 bytes of the key reverse it and XOR with v0,v2
  memcpy(m, key,8);
  reverse64(m);
  xor64(v0, m);
  xor64(v2, m);

  memcpy(m, key+8,8);
  reverse64(m);
  xor64(v1, m);
  xor64(v3, m);
  
  // process a single block
  int i=0;
  for (; i<8; i++) { m[i] = block[7-i] ^ hash[7-i]; }
  xor64(v3, m);
  siphash_round();  
  siphash_round();  
  xor64(v0, m);
  
  // Finalize hash
  xor_ff(v2);
  siphash_round();
  siphash_round();
  siphash_round();
  siphash_round();
  xor64(v0, v1);
  xor64(v0, v2);
  xor64(v0, v3);
  
  // store hash
  memcpy(hash, v0, 8);
  reverse64(hash);
}




/** Computes the CBC-MAC hash from the inlen bytes stored in @c in using the @c key and the current
 * @c hash value as the IV. The @c hash gets updated constantly. */
void siphash24_cbc_mac(unsigned char *hash,
                       const unsigned char *in, unsigned int inlen, const unsigned char *key) 
{  
  unsigned int rem = inlen;
  // Process data in 64bit blocks
  while (rem >= 8) {
    siphash24_hash_block(hash, in, key);
    rem -= 8; in += 8;
  }
  
  unsigned char block[8];
  unsigned int i=0;
  // Add remaining elements
  for (; i<rem; i++) { block[i] = in[i]; }
  // 0-pad to 7 bytes
  for (i=rem; i<7; i++) { block[i] = 0; }
  // Store length % 256 as 8-th byte
  block[7] = inlen;
  siphash24_hash_block(hash, block, key);
}
