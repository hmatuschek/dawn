#ifndef __SIP_HASH_2_4_H__
#define __SIP_HASH_2_4_H__

#ifdef __cplusplus
extern "C" {
#endif

/** CBC-MAC implementation using SipHash 2-4.
 * @param hash (64 bit) On entry, it contains the current IV (last hash). On exit it contains the
 *        hash (next IV).
 * @param in Input buffer.
 * @param inlen Input buffer length.
 * @param key (128 bit) Shared secret.*/
void siphash24_cbc_mac(
    unsigned char *hash, const unsigned char *in, unsigned long long inlen, const unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif  // __SIP_HASH_2_4_H__
