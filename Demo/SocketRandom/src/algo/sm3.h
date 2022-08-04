#ifndef _SM3_H_
#define _SM3_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
#include "global.h"
/* SM3 context. */
typedef struct {
  UINT4 state[8];                                   /* state (ABCDEFGH) */
  UINT4 count[2];                                   /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} SM3_CTX;


void SM3Init PROTO_LIST ((SM3_CTX *));
void SM3Update PROTO_LIST((SM3_CTX *, unsigned char *, unsigned int));
void SM3Final PROTO_LIST ((unsigned char [32], SM3_CTX *));
int SM3_Hash_NOID(unsigned char * inputData,int inputDataLen,unsigned char *outputData);
int SM3_Hash_ID(unsigned char * XY,int XYLen,unsigned char * inputData,int inputDataLen,unsigned char *outputData);

#ifdef __cplusplus
}
#endif

#endif

