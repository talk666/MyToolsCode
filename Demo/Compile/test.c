#include <stdio.h>
#include <string.h>

#include "xprint.h"

#include <sys/time.h>
#include <time.h>

size_t OPENSSL_ia32_rdseed_bytes(unsigned char *buf, size_t len);
size_t OPENSSL_ia32_rdrand_bytes(unsigned char *buf, size_t len);

#define RAND_FILE_SIZE (10 * 1024 * 1024)

int main() {

    unsigned char buffer[32] = {0};
    int            i;
    int            ret;
    size_t bytes_needed = 32;
    size_t rand = 0;

    struct timeval time1, time2;
    uint64_t       ts1, ts2, ts;

    FILE *fp;

    gettimeofday(&time1, NULL);

    fp = fopen("srand.bin", "w");
    if (fp == NULL) {
        ferror(fp);
        return -1;
    }
  //  xprint_hex("Random0:\r\n", 0, buffer, 32);

//. byte 73,15,199,250 error  指令集不存在
    // if (OPENSSL_ia32_rdseed_bytes(buffer, bytes_needed) == bytes_needed) {
    //     printf("OPENSSL_ia32_rdseed_bytes\r\n");
    // }

    // xprint_hex("Random1:\r\n", 0, buffer, 32);
    // memset(buffer, 0, 32);
    
    for (i = 0; i < RAND_FILE_SIZE / 32; i++) {
        ret = OPENSSL_ia32_rdrand_bytes(buffer, 32);
        if (ret != 32) {
            printf("error %x\n", ret);
            return -1;
        }

        fwrite(buffer, 1, 32, fp);
    }

    fclose(fp);

    gettimeofday(&time2, NULL);

    ts1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
    ts2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;

    ts = ts2 - ts1;

    printf("time left = %.2fs\n", ts / 1000.0);
    printf("speed = %dMbps\n", RAND_FILE_SIZE / 1024 / 1024 * 8 * 1000 / ts);

    // if (OPENSSL_ia32_rdrand_bytes(buffer, bytes_needed) == bytes_needed) {
    //    // printf("OPENSSL_ia32_rdrand_bytes\r\n");
    // }

  //  xprint_hex("Random2:\r\n", 0, buffer, 32);

    return 0;
}