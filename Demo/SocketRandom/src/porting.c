#include "app.h"

#include <stdint.h>
#include <sys/time.h>
#include <time.h>


uint64_t __get_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

// 获得熵源
int __get_entropy(uint8_t *data, uint32_t len)
{

#if 0
    FILE *fp;

    fp = fopen("/dev/urandom", "r");
    if (fp == NULL) {
        return -1;
    }

    fread(data, 1, len, fp);

    fclose(fp);
#else
    struct timeval tv;
    uint32_t       us;

    gettimeofday(&tv, NULL);

    us = tv.tv_usec;
    memcpy(data, &us, 4);
#endif

    return 0;
}
