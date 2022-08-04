#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "sm3.h"

#include "xprint.h"

/*常量*/

const int T[64] = { 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
                    0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a,
                    0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a };
/*布尔函数*/

#define FF1(x, y, z) ((x) ^ (y) ^ (z))
#define FF2(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG1(x, y, z) ((x) ^ (y) ^ (z))
#define GG2(x, y, z) (((x) & (y)) | ((~x) & (z)))

/* Cyclic_LEFT cyclic x left n bits.*/

#define Cyclic_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/*置换函数*/

#define P0(x) ((x) ^ (Cyclic_LEFT(x, 9)) ^ (Cyclic_LEFT(x, 17)))  /*压缩函数中的置换函数*/
#define P1(x) ((x) ^ (Cyclic_LEFT(x, 15)) ^ (Cyclic_LEFT(x, 23))) /*消息扩展中的置换函数*/

void                  SM3Transform(unsigned int *, unsigned char *);
void                  MessageExtend(unsigned char *buffer, unsigned int *W0, unsigned int *W1);
static void Encode    PROTO_LIST((unsigned char *, UINT4 *, unsigned int));
static void Decode    PROTO_LIST((UINT4 *, unsigned char *, unsigned int));
static void SM3memcpy PROTO_LIST((POINTER, POINTER, unsigned int));
static void SM3memset PROTO_LIST((POINTER, int, unsigned int));
static unsigned char  PADDING[64] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* SM3 initialization. Begins an SM3 operation, writing a new context. */

void SM3Init(SM3_CTX *context) /* context */
{
    context->count[0] = context->count[1] = 0;

    /* Load magic initialization constants.   */
    context->state[0] = 0x7380166f;
    context->state[1] = 0x4914b2b9;
    context->state[2] = 0x172442d7;
    context->state[3] = 0xda8a0600;
    context->state[4] = 0xa96f30bc;
    context->state[5] = 0x163138aa;
    context->state[6] = 0xe38dee4d;
    context->state[7] = 0xb0fb0e4e;
}

/* SM3 block update operation. Continues an SM3 message-digest
   operation, processing another message block, and updating the
   context.*/

void SM3Update(SM3_CTX *context, unsigned char *input, unsigned int inputLen)
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((context->count[1] >> 3) & 0x3F);

    /* Update number of bits */
    if ((context->count[1] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
        context->count[0]++;
    context->count[0] += ((UINT4)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible.*/
    if (inputLen >= partLen) {
        SM3memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        SM3Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            SM3Transform(context->state, &input[i]);

        index = 0;
    } else
        i = 0;

    /* Buffer remaining input */
    SM3memcpy((POINTER)&context->buffer[index], (POINTER)&input[i], inputLen - i);
}

/* SM3 finalization. Ends an SM3 message-digest operation, writing the
     the message digest and zeroizing the context. */

void SM3Final(unsigned char digest[32], SM3_CTX *context)
{
    unsigned char bits[8];
    unsigned int  index, padLen;

    /* Save number of bits */
    Encode(bits, context->count, 8);

    /* Pad out to 56 mod 64.*/
    index = (unsigned int)((context->count[1] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);

    SM3Update(context, PADDING, padLen);

    /* Append length (before padding) */
    SM3Update(context, bits, 8);

    /* Store state in digest */
    Encode(digest, context->state, 32);

    /* Zeroize sensitive information.*/
    SM3memset((POINTER)context, 0, sizeof(*context));
}

/* SM3 basic transformation. Transforms state based on block.*/

void SM3Transform(UINT4 *state, unsigned char *buffer)
{

    int          i;
    UINT4        SS1, SS2, TT1, TT2, SS;
    UINT4        W0[68];
    UINT4        W1[64];
    unsigned int intMsg[16];
    UINT4        A = state[0], B = state[1], C = state[2], D = state[3], E = state[4], F = state[5], G = state[6],
          H = state[7];


    Decode(intMsg, buffer, 64);
    MessageExtend((unsigned char *)intMsg, W0, W1);

    for (i = 0; i < 64; i++) {

        SS = Cyclic_LEFT(A, 12) + E + Cyclic_LEFT(T[i], i);
        SS1 = Cyclic_LEFT(SS, 7);
        SS2 = SS1 ^ Cyclic_LEFT(A, 12);

        if (i >= 0 && i <= 15) {
            TT1 = FF1(A, B, C) + D + SS2 + W1[i];
            TT2 = GG1(E, F, G) + H + SS1 + W0[i];
        } else {
            TT1 = FF2(A, B, C) + D + SS2 + W1[i];
            TT2 = GG2(E, F, G) + H + SS1 + W0[i];
        }

        D = C;
        C = Cyclic_LEFT(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = Cyclic_LEFT(F, 19);
        F = E;
        E = P0(TT2);

        /*
                        printf("loop[%d]:\nA: %08x\n", i,A);
                        printf("B: %08x\n", B);
                        printf("C: %08x\n", C);
                        printf("D: %08x\n", D);
                        printf("E: %08x\n", E);
                        printf("F: %08x\n", F);
                        printf("G: %08x\n", G);
                        printf("H: %08x\n", H);
                        printf("\n");
        */
    }
    state[0] ^= A;
    state[1] ^= B;
    state[2] ^= C;
    state[3] ^= D;
    state[4] ^= E;
    state[5] ^= F;
    state[6] ^= G;
    state[7] ^= H;

    /*
       printf("After\n%08x\n",state[0]);
       printf("%08x\n", state[1]);
       printf("%08x\n", state[2]);
       printf("%08x\n", state[3]);
       printf("%08x\n", state[4]);
       printf("%08x\n", state[5]);
       printf("%08x\n", state[6]);
       printf("%08x\n", state[7]);
       printf("\n");
     */
}

/*消息扩展*/

void MessageExtend(unsigned char *buffer, unsigned int *W0, unsigned int *W1)
{
    int          i, j;
    unsigned int W;

    for (i = 0; i < 16; i++)
        SM3memcpy((unsigned char *)(W0 + i), &buffer[i * 4], 4);

    for (i = 16; i < 68; i++) {
        W = (W0[i - 16]) ^ (W0[i - 9]) ^ (Cyclic_LEFT((W0[i - 3]), (15)));
        W = P1(W);
        W0[i] = (W) ^ (Cyclic_LEFT(W0[i - 13], 7)) ^ (W0[i - 6]);
    }

    for (j = 0; j < 64; j++) {
        W1[j] = W0[j] ^ W0[j + 4];
    }
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
     a multiple of 4. */

static void  Encode(output, input, len) unsigned char *output;
UINT4       *input;
unsigned int len;
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j + 3] = (unsigned char)(input[i] & 0xff);
        output[j + 2] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j + 1] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
     a multiple of 4. */

static void    Decode(output, input, len) UINT4 *output;
unsigned char *input;
unsigned int   len;
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j + 3]) | (((UINT4)input[j + 2]) << 8) | (((UINT4)input[j + 1]) << 16) |
                    (((UINT4)input[j]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.*/

static void  SM3memcpy(output, input, len) POINTER output;
POINTER      input;
unsigned int len;
{
    unsigned int i;

    for (i = 0; i < len; i++)
        output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible. */

static void  SM3memset(output, value, len) POINTER output;
int          value;
unsigned int len;
{
    unsigned int i;

    for (i = 0; i < len; i++)
        ((char *)output)[i] = (char)value;
}

int ECF_SCH(unsigned char *XY, int XYLen, unsigned char *pOutdata)
{
    unsigned char pID[] = "1234567812345678";
    int           nIDLen = 16;
    unsigned char Indata[300];
    unsigned char ga[32] = { 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
                             0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC };
    unsigned char gb[32] = { 0x28, 0xE9, 0xFA, 0x9E, 0x9D, 0x9F, 0x5E, 0x34, 0x4D, 0x5A, 0x9E,
                             0x4B, 0xCF, 0x65, 0x09, 0xA7, 0xF3, 0x97, 0x89, 0xF5, 0x15, 0xAB,
                             0x8F, 0x92, 0xDD, 0xBC, 0xBD, 0x41, 0x4D, 0x94, 0x0E, 0x93 };
    unsigned char gx[32] = { 0x32, 0xC4, 0xAE, 0x2C, 0x1F, 0x19, 0x81, 0x19, 0x5F, 0x99, 0x04,
                             0x46, 0x6A, 0x39, 0xC9, 0x94, 0x8F, 0xE3, 0x0B, 0xBF, 0xF2, 0x66,
                             0x0B, 0xE1, 0x71, 0x5A, 0x45, 0x89, 0x33, 0x4C, 0x74, 0xC7 };
    unsigned char gy[32] = { 0xBC, 0x37, 0x36, 0xA2, 0xF4, 0xF6, 0x77, 0x9C, 0x59, 0xBD, 0xCE,
                             0xE3, 0x6B, 0x69, 0x21, 0x53, 0xD0, 0xA9, 0x87, 0x7C, 0xC6, 0x2A,
                             0x47, 0x40, 0x02, 0xDF, 0x32, 0xE5, 0x21, 0x39, 0xF0, 0xA0 };
    unsigned char len[2];
    int           rv;

    memset(len, 0, 2);
    len[0] = (unsigned char)(nIDLen * 8 >> 8);
    len[1] = (unsigned char)nIDLen * 8;
    memcpy(Indata, len, 2);
    memcpy(Indata + 2, pID, nIDLen);
    memcpy(Indata + 2 + nIDLen, ga, 32);
    memcpy(Indata + 2 + nIDLen + 32, gb, 32);
    memcpy(Indata + 2 + nIDLen + 32 + 32, gx, 32);
    memcpy(Indata + 2 + nIDLen + 32 + 32 + 32, gy, 32);
    memcpy(Indata + 2 + nIDLen + 32 + 32 + 32 + 32, XY, XYLen);

    SM3_Hash_NOID(Indata, 2 + nIDLen + 32 + 32 + 32 + 32 + XYLen, pOutdata); // xAyALen=64
}

int SM3_Hash_NOID(unsigned char *inputData, int inputDataLen, unsigned char *outputData)
{
    SM3_CTX context;

    if (inputData == NULL || inputDataLen < 1) {
        return 2;
    }
    if (outputData == NULL) {
        return 3;
    }
    SM3Init(&context);
    SM3Update(&context, inputData, inputDataLen);
    SM3Final(outputData, &context);
}

int SM3_Hash_ID(unsigned char *XY, int XYLen, unsigned char *inputData, int inputDataLen, unsigned char *outputData)
{
    unsigned char IDData[100] = { 0 };
    SM3_CTX       context;
    if (XY == NULL || XYLen != 64) {
        return 1;
    }
    if (inputData == NULL || inputDataLen < 1) {
        return 2;
    }
    if (outputData == NULL) {
        return 3;
    }
    ECF_SCH(XY, XYLen, IDData);

    SM3Init(&context);
    SM3Update(&context, IDData, 32);
    SM3Update(&context, inputData, inputDataLen);
    SM3Final(outputData, &context);
}
