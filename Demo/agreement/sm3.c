/* ============================================================================
 * Copyright (c) 2010-2015.  All rights reserved.
 * SM3 Hash Cipher Algorithm: Digest length is 256-bit
 * ============================================================================
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sm3.h"

#define ROTL(x,y) ((((x)<<(y&(32-1))) | ((x)>>(32-(y&(32-1))))) & 0xffffffff)

static void sm3_ms(uint32_t *pW, const uint32_t *pMsg)
{
    int32_t j;
	
    for (j = 0; j < 16; j++)
    {
        pW[j] = pMsg[j];
    }

    for (; j < 68; j++)
    {
        pW[j] = pW[j-16] ^ pW[j-9] ^ ROTL(pW[j-3], 15);
        pW[j] ^= (ROTL(pW[j], 15) ^ ROTL(pW[j], 23));
        pW[j] ^= ((ROTL(pW[j-13], 7) ^ pW[j-6]));
    }

    for (j = 0; j < 64; j++)
    {
        pW[68+j] = pW[j] ^ pW[j+4];
    }
}

static void sm3_cf(uint32_t *pH, const uint8_t *pMsg)
{
    uint32_t A = pH[0], B = pH[1], C = pH[2], D = pH[3], E = pH[4], F = pH[5], G = pH[6], H = pH[7];
    uint32_t w[68+64];

    // w[0..67] for W
    // w[68+0 .. 68+63] for W'

    int32_t j;
    // unsigned char --> uint32_t
    uint32_t BB[16];
    for (j = 0; j < 16; j++)
    {
        BB[j] = (pMsg[4*j] << 24) | (pMsg[4*j+1] << 16) | (pMsg[4*j+2] << 8) | pMsg[4*j+3];
    }

    sm3_ms(w, BB);

    uint32_t SS1, SS2, TT1, TT2;
    uint32_t T = 0x79CC4519;
    for (j = 0; j < 16; j++)
    {
        SS1 = (ROTL(A, 12) + E + ROTL(T, j))%0x100000000;
        SS1 = ROTL(SS1, 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = (((A^B^C) + D + SS2) + w[j+68])%0x100000000;
        TT2 = (((E^F^G) + H + SS1) + w[j])%0x100000000;
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = TT2 ^ ROTL(TT2, 9) ^ ROTL(TT2, 17);
    }

    T = 0x7A879D8A;
    for (; j < 64; j++)
    {
        SS1 = (ROTL(A, 12) + E + ROTL(T, j))%0x100000000;
        SS1 = ROTL(SS1, 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = ((((A&B) | (A&C) | (B&C)) + D + SS2) + w[j+68])%0x100000000;
        TT2 = ((((E&F) | ((~E)&G)) + H + SS1) + w[j])%0x100000000;
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = TT2 ^ ROTL(TT2, 9) ^ ROTL(TT2, 17);
    }

    pH[0] ^= A;
    pH[1] ^= B;
    pH[2] ^= C;
    pH[3] ^= D;
    pH[4] ^= E;
    pH[5] ^= F;
    pH[6] ^= G;
    pH[7] ^= H;
}

int32_t sm3_init(struct sm3_ctx *ctx)
{
    ctx->h[0] = 0x7380166F;
    ctx->h[1] = 0x4914B2B9;
    ctx->h[2] = 0x172442D7;
    ctx->h[3] = 0xDA8A0600;
    ctx->h[4] = 0xA96F30BC;
    ctx->h[5] = 0x163138AA;
    ctx->h[6] = 0xE38DEE4D;
    ctx->h[7] = 0xB0FB0E4E;

    ctx->len = 0;

    return 0;
}

int32_t sm3_update(struct sm3_ctx *ctx, const uint8_t *in, uint32_t inlen)
{
    uint32_t i, iCurPos = (uint32_t)(ctx->len % 64);
    uint32_t iMin = (64-iCurPos < inlen) ? (64-iCurPos) : (inlen);

    for (i = 0; i < iMin; i++)
    {
        ctx->bb[i+iCurPos] = in[i];
    }
	
    ctx->len += inlen;

    if (inlen >= 64-iCurPos)
    {
        sm3_cf(ctx->h, ctx->bb);

        inlen -= (64-iCurPos);
        in += (64-iCurPos);
        for (i = 0; i < inlen/64; i++)
        {
            sm3_cf(ctx->h, in+i*64);
        }

        for (i = 0; i < (inlen & 63); i++)
        {
            ctx->bb[i] = in[(inlen&0xffffffc0)+i];
        }
    }

    return 0;
}

int32_t sm3_final(struct sm3_ctx *ctx, uint8_t *hash)
{
    uint32_t i;
    uint8_t bb[128];

    for (i = 0; i < sizeof(bb); i++)
    {
        bb[i] = 0x00;
    }

    for (i = 0; i < (ctx->len & 63); i++)
    {
        bb[i] = ctx->bb[i];
    }

    uint32_t low = 0, high = 0;
    high = (uint32_t)(ctx->len >> 29);
    high = (high >> 24) | ((high&0x00ff0000) >> 8) | ((high&0x0000ff00) << 8) | (high << 24);
    low = (uint32_t)(ctx->len << 3);
    low = (low >> 24) | ((low&0x00ff0000) >> 8) | ((low&0x0000ff00) << 8) | (low << 24);

    if ((ctx->len & 63) < 56)
    {
        // 1 block
        bb[i] = 0x80;

        // BB[56 .. 63]
        *(uint32_t *)(bb+56) = high;
        *(uint32_t *)(bb+60) = low;

        sm3_cf(ctx->h, bb);
    }
    else
    {
        // 2 block
        bb[i] = 0x80;
        sm3_cf(ctx->h, bb);

        // BB[120 .. 127]
        *(uint32_t *)(bb+120) = high;
        *(uint32_t *)(bb+124) = low;

        sm3_cf(ctx->h, bb+64);
    }

    for (i = 0; i < 8; i++)
    {
        hash[i*4] = (unsigned char)(ctx->h[i] >> 24);
        hash[i*4+1] = (unsigned char)(ctx->h[i] >> 16);
        hash[i*4+2] = (unsigned char)(ctx->h[i] >> 8);
        hash[i*4+3] = (unsigned char)(ctx->h[i]);
    }

    return 0;
}

int32_t sm3(const uint8_t *in, uint32_t inlen, uint8_t *hash)
{
    uint32_t i;

    uint32_t H[8] =
    {
        0x7380166F,
        0x4914B2B9,
        0x172442D7,
        0xDA8A0600,
        0xA96F30BC,
        0x163138AA,
        0xE38DEE4D,
        0xB0FB0E4E,
    };

    for (i = 0; i < inlen/64; i++)
    {
        sm3_cf(H, in+i*64);
    }

    unsigned char BB2[128];
    for (i = 0; i < sizeof(BB2); i++)
    {
        BB2[i] = 0x00;
    }

    for (i = 0; i < (inlen & 63); i++)
    {
        BB2[i] = in[(inlen&0xffffffc0)+i];
    }

    // in bytes ==> in bits
    uint32_t low = 0, high = 0;
    high = inlen >> 29;
    high = (high >> 24) | ((high&0x00ff0000) >> 8) | ((high&0x0000ff00) << 8) | (high << 24);
    low = inlen << 3;
    low = (low >> 24) | ((low&0x00ff0000) >> 8) | ((low&0x0000ff00) << 8) | (low << 24);

    if ((inlen & 63) < 56)
    {
        // 1 block
        BB2[i] = 0x80;

        // BB[56 .. 63]
        *(uint32_t *)(BB2+56) = high;
        *(uint32_t *)(BB2+60) = low;

        sm3_cf(H, BB2);
    }
    else
    {
        // 2 block
        BB2[i] = 0x80;
        sm3_cf(H, BB2);

        // BB[120 .. 127]
        *(uint32_t *)(BB2+120) = high;
        *(uint32_t *)(BB2+124) = low;

        sm3_cf(H, BB2+64);
    }

    for (i = 0; i < 8; i++)
    {
        hash[i*4] = (unsigned char)(H[i] >> 24);
        hash[i*4+1] = (unsigned char)(H[i] >> 16);
        hash[i*4+2] = (unsigned char)(H[i] >> 8);
        hash[i*4+3] = (unsigned char)H[i];
    }

    return 0;
}


