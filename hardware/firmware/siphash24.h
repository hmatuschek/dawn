#ifndef __SIP_HASH_2_4_H__
#define __SIP_HASH_2_4_H__

#ifdef __cplusplus
extern "C" {
#endif

void siphash24_mac(
    unsigned char *hash, const unsigned char *in, unsigned int inlen, const unsigned char *key);

void siphash24_cbc_mac(
    unsigned char *hash, const unsigned char *in, unsigned int inlen, const unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif  // __SIP_HASH_2_4_H__
