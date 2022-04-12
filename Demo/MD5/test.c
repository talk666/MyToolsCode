#include <stdio.h>
#include "md5.h"

int  main()
{
  
      const unsigned char data[7]="ABCDEF";
      unsigned char md[16]={0};

 //MD5(data,strlen((const char *)data),md);
      MD5(data, 6, md);
      int i;
      for(i = 0;i < 16;i++) {
        printf("%02x-", *(md + i));
    }
    return 0;

}
