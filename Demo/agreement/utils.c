#include <stdint.h>
#include <string.h>

int hex_string_to_array(const char *inStr, unsigned char *outArray, unsigned int *outArrayLen)
{
    unsigned int  i, k = 0;
    unsigned char HByte, LByte;
    unsigned int  inStrLen;


    inStrLen = strlen(inStr);
    if (inStrLen % 2 != 0)
        return -1;

    for (i = 0; i < inStrLen; i = i + 2) {
        if (inStr[i] >= '0' && inStr[i] <= '9') {
            HByte = inStr[i] - '0';
        } else if (inStr[i] >= 'A' && inStr[i] <= 'F') {
            HByte = inStr[i] + 10 - 'A';
        } else if (inStr[i] >= 'a' && inStr[i] <= 'f') {
            HByte = inStr[i] + 10 - 'a';
        } else {
            HByte = inStr[i];
            return -1;
        }
        HByte = HByte << 4;
        HByte = HByte & 0xF0;
        if (inStr[i + 1] >= '0' && inStr[i + 1] <= '9') {
            LByte = inStr[i + 1] - '0';
        } else if (inStr[i + 1] >= 'A' && inStr[i + 1] <= 'F') {
            LByte = inStr[i + 1] + 10 - 'A';
        } else if (inStr[i + 1] >= 'a' && inStr[i + 1] <= 'f') {
            LByte = inStr[i + 1] + 10 - 'a';
        } else {
            LByte = inStr[i];
            return -1;
        }
        outArray[k++] = HByte | LByte;
    }

    *outArrayLen = k;

    return 0;
}