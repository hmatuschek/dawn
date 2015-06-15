#ifndef __SIP_HASH_2_4_H__
#define __SIP_HASH_2_4_H__

void siphash24_cbc_mac(
    unsigned char *hash, const unsigned char *in, unsigned int inlen, const unsigned char *key);

#endif  // __SIP_HASH_2_4_H__
