
#include <stdio.h>
#include <string.h>
#include "md5.h"


unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md)
{
    MD5_CTX c;
    static unsigned char m[MD5_DIGEST_LENGTH];

    if (md == NULL)
        md = m;
    if (!MD5_Init(&c))
        return NULL;

    MD5_Update(&c, d, n);

    MD5_Final(md, &c);
    
    memset(&c, 0, sizeof(c));
    return md;
}
