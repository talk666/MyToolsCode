#include <stdio.h>
#include "md5.h"

int  main()
{
  while (1) {

    const unsigned char data[] = {};
    printf("insert string\r\n");
    scanf("%s",data);

    printf("len = %d\r\n", strlen(data));
    unsigned char md[16] = { 0 };


    MD5(data, strlen(data), md);
    int i;
    printf("string: %s, MD5:",data);
    for (i = 0;i < 16;i++) {
      printf("%02X", *(md + i));
    }
    
    printf("\r\n\r\n\r\n");
    usleep(5000);
  }
    return 0;
}
