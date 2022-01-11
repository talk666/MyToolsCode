/* ============================================================================
 * Copyright (c) 2010-2015.  All rights reserved.
 * SM3 Hash Cipher Algorithm: Digest length is 256-bit
 * ============================================================================
 */

#ifndef __SM3_HEADER__
#define __SM3_HEADER__

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SM3_HASH_SIZE 32

typedef struct sm3_ctx
{
    uint32_t h[8];
    uint8_t bb[64];
    uint64_t len;
} sm3_ctx_t;

int32_t sm3_init(struct sm3_ctx *ctx);
int32_t sm3_update(struct sm3_ctx *ctx, const uint8_t *in, uint32_t inlen);
int32_t sm3_final(struct sm3_ctx *ctx, uint8_t *hash);
int32_t sm3(const uint8_t *in, uint32_t inlen, uint8_t *hash);


#ifdef  __cplusplus
}
#endif

#endif
