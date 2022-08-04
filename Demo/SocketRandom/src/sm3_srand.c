#include "app.h"
#include "porting.h"
#include "sm3.h"

// entropy
#define ENTROPY_POOL_SIZE (512)
//entropy_input_len
#define ENTROPY_INPUT_SIZE (10)
typedef struct
{
    uint8_t  V[ENTROPY_POOL_SIZE];
    uint8_t  C[ENTROPY_POOL_SIZE];
    uint64_t reseedLastTime; // 上次重播种时间
    uint32_t outCnt;         // 输出计数
    uint32_t outMaxTimes;    // 输出最大次数
    uint64_t outMaxInterval; // 输出最长间隔
    uint32_t pool[ENTROPY_POOL_SIZE / 4];
} srand_sm3_t;

srand_sm3_t srand_sm3;

uint32_t table[] = { 0x00, 0xAF6B3073, 0xD9700E70, 0xDD7E22B6, 0x4636AE04, 0xB5C51761, 0x9F6E2C9F, 0xD302552D };

//sm3接口
static int __calc_df(uint8_t *data, uint32_t len, uint8_t *out)
{
    SM3_CTX ctx;
    uint8_t hash[32];

    if (data == NULL || len == 0) {
        return SRAND_SM3_ERR_PARAM;
    }

    SM3Init(&ctx);
    SM3Update(&ctx, data, len);
    SM3Final(hash, &ctx);

    memcpy(out, hash, 32);

    return 0;
}

//新的熵数g  更新pool
static int __calc_pool(uint8_t *buff)
{
    uint32_t  g;
    int       j;
    uint32_t *data;
    uint32_t  temp;

    data = (uint32_t *)buff;

    __get_entropy((uint8_t *)&g, sizeof(g));

    for (j = 0; j < 128; j++) {
        temp = g ^ data[j];
        temp = temp ^ data[(j + 1) % 128];
        temp = temp ^ data[(j + 25) % 128];
        temp = temp ^ data[(j + 51) % 128];
        temp = temp ^ data[(j + 76) % 128];
        temp = temp ^ data[(j + 103) % 128];
        temp = (temp >> 3) ^ table[temp & 7];
        data[j] = temp;
    }

    return 0;
}

//srand初始化接口
int srand_sm3_init(uint8_t *pnonce, uint16_t nonceLen)
{
    uint8_t *data;

    if (pnonce == NULL || nonceLen == 0) {
        return SRAND_SM3_ERR_PARAM;
    }

    if (nonceLen > 512) {
        return SRAND_SM3_ERR_PARAM;
    }

    data = malloc(nonceLen + ENTROPY_POOL_SIZE);
    if (data == NULL) {
        return SRAND_SM3_ERR_MALLOC;
    }

    memset(&srand_sm3, 0, sizeof(srand_sm3));

//熵池
    __calc_pool((uint8_t *)srand_sm3.pool);

    memcpy(data, srand_sm3.pool, ENTROPY_POOL_SIZE);
    memcpy(data + ENTROPY_POOL_SIZE, pnonce, nonceLen);

//sm3 data的hash结果
    __calc_df(data, nonceLen + ENTROPY_POOL_SIZE, srand_sm3.V);

//当前播种时间
    srand_sm3.reseedLastTime = __get_time();

    srand_sm3.outCnt = 1;
    srand_sm3.outMaxTimes = 1024;
//重播种时间为60s
    srand_sm3.outMaxInterval = 60;

    free(data);

    return 0;
}

//重播种接口
int srand_sm3_reseed()
{   
    uint8_t *data;
    data = malloc(1 + ENTROPY_INPUT_SIZE + ENTROPY_POOL_SIZE);
    if (data == NULL) {
        return SRAND_SM3_ERR_MALLOC;
    }
    __calc_pool((uint8_t *)srand_sm3.pool);

    data[0] = 0x01;
    memcpy(data + 1, "1593577811", ENTROPY_INPUT_SIZE);
    memcpy(data + 1 + ENTROPY_INPUT_SIZE, srand_sm3.pool, ENTROPY_POOL_SIZE);

    __calc_df(data, 1 + ENTROPY_INPUT_SIZE + ENTROPY_POOL_SIZE, srand_sm3.V);

    srand_sm3.reseedLastTime = __get_time();

    srand_sm3.outCnt = 1;

    return 0;
}

//srand生成接口
int srand_sm3_generate(uint32_t len, uint8_t *out)
{
    int      reseedFlag = 0;
    uint64_t nowTime = 0;
    uint8_t *data;
    uint8_t  hash[32];

    if (out == NULL) {
        return SRAND_SM3_ERR_PARAM;
    }

    if (len > 32) {
        return SRAND_SM3_ERR_PARAM;
    }

    if (srand_sm3.outCnt >= srand_sm3.outMaxTimes) {
        reseedFlag = 1;
    }

    nowTime = __get_time();
    nowTime = nowTime - srand_sm3.reseedLastTime;
    nowTime = nowTime / 1000000;

    if (nowTime >= srand_sm3.outMaxInterval) {
        reseedFlag = 1;
    }

    if (reseedFlag) {
        srand_sm3_reseed();
    }

    data = malloc(1 + 32 + ENTROPY_POOL_SIZE);
    if (data == NULL) {
        return SRAND_SM3_ERR_MALLOC;
    }

    data[0] = 0x02;
    memcpy(data + 1, "147258369", 9);
    memcpy(data + 1 + 32, srand_sm3.V, ENTROPY_POOL_SIZE);

    __calc_df(data, 1 + 32 + ENTROPY_POOL_SIZE, hash);

//更新V值
    memcpy(srand_sm3.V, hash, 32);

    free(data);

    memcpy(out, hash, len);

    srand_sm3.outCnt++;

    return 0;
}
