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

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

#define ROTL(x,b) (u64)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define U32TO8_LE(p, v)         \
    (p)[0] = (u8)((v)      ); (p)[1] = (u8)((v) >>  8); \
    (p)[2] = (u8)((v) >> 16); (p)[3] = (u8)((v) >> 24);

#define U64TO8_LE(p, v)         \
  U32TO8_LE((p),     (u32)((v)      ));   \
  U32TO8_LE((p) + 4, (u32)((v) >> 32));

#define U8TO64_LE(p) \
  (((u64)((p)[0])      ) | \
   ((u64)((p)[1]) <<  8) | \
   ((u64)((p)[2]) << 16) | \
   ((u64)((p)[3]) << 24) | \
   ((u64)((p)[4]) << 32) | \
   ((u64)((p)[5]) << 40) | \
   ((u64)((p)[6]) << 48) | \
   ((u64)((p)[7]) << 56))

#define SIPROUND            \
  do {              \
    v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
    v2 += v3; v3=ROTL(v3,16); v3 ^= v2;     \
    v0 += v3; v3=ROTL(v3,21); v3 ^= v0;     \
    v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
  } while(0)


/* SipHash-2-4 */
void
siphash24_hash(uint8_t *hash, const uint8_t *block, const uint8_t *k)
{
  /* "somepseudorandomlygeneratedbytes" */
  u64 v0 = 0x736f6d6570736575ULL;
  u64 v1 = 0x646f72616e646f6dULL;
  u64 v2 = 0x6c7967656e657261ULL;
  u64 v3 = 0x7465646279746573ULL;
  u64 k0 = U8TO64_LE( k );
  u64 k1 = U8TO64_LE( k + 8 );
  u64 m;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

  m = U8TO64_LE( block ) ^ U8TO64_LE(hash);
  v3 ^= m;
  SIPROUND;
  SIPROUND;
  v0 ^= m;

  v2 ^= 0xff;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  U64TO8_LE( hash, v0 ^ v1 ^ v2 ^ v3 );
}


/** Computes the CBC-MAC from the @c inlen bytes stored in @c in using the @c key and the
 * current @c hash value as the IV. The @c hash gets updated constantly. */
void siphash24_cbc_mac(uint8_t *hash, const uint8_t *in, uint8_t inlen, const uint8_t *key)
{
  // Process first 64-bit blocks
  uint8_t rem = inlen;
  while (rem >= 8) {
    siphash24_hash(hash, in, key);
    rem -= 8; in += 8;
  }

  unsigned char block[8];
  // store remaining data
  memcpy(block, in, rem);
  // 0-pad that up to 7-bytes
  memset(block+rem, 0, 7-rem);
  // store inlen mod 256 at last byte
  block[7] = inlen;
  // Last hash
  siphash24_hash(hash, block, key);
}
