#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRAND_SM3_ERR_OK     0
#define SRAND_SM3_ERR_PARAM  1
#define SRAND_SM3_ERR_MALLOC 2

int srand_sm3_init(uint8_t *pnonce, uint16_t nonceLen);

int srand_sm3_reseed();

int srand_sm3_generate(uint32_t len, uint8_t *out);

#endif