#ifndef __SIP_HASH_2_4_H__
#define __SIP_HASH_2_4_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** CBC-MAC implementation using SipHash 2-4.
 * @param hash (64 bit) On entry, it contains the current IV (last hash). On exit it contains the
 *        hash (next IV).
 * @param in Input buffer.
 * @param inlen Input buffer length.
 * @param key (128 bit) Shared secret.*/
void siphash24_cbc_mac(uint8_t *hash, const uint8_t *in, size_t inlen, const uint8_t *key);

#ifdef __cplusplus
}
#endif

#endif  // __SIP_HASH_2_4_H__
