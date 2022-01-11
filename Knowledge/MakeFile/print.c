#include <stdio.h>
#include <string.h>

// extern void debug_printf(const char *fmt, ...);
#define PRINT_HEX_PRINTF(...) printf(__VA_ARGS__)

// [send]
// 0000: 7E 25 02 00 00 7D 02 02  00 00 2B 48 54 00 47 56  |~%...}.. ..+HT.GV|
// 0010: 35 55 5C 00 00 00 00 00  00 00 00 00 00 00 00 00  |5U\..... ........|
// 0020: 00 00 00 00 00 00 00 00  00 20 01 14 16 31 00 E1  |........ . ...1..|
// 0030: 0D 02 00 0F 0A 00 F0 58  53 15 0C DA BD 05 61 7D  |.......X S.....a}|
// 0040: 02 E8 5A 7F                                       |..Z.             |
// DEC:00068 HEX:0044
void print_hex(char *title, int offset, unsigned char *data, int dataLen)
{
    int   i = 0;
    int   lineLen;
    char  tmpBuf[20];
    char *buf;
    int   len = dataLen;

    if (title != NULL) {
        PRINT_HEX_PRINTF("%s", title);
    }

    while (1) {
        buf = tmpBuf;

        lineLen = (dataLen > 16 ? 16 : dataLen);
        PRINT_HEX_PRINTF("%06X:", offset);

        for (i = 0; i < 16; i++) {
            if (i < lineLen) {
                PRINT_HEX_PRINTF(" %02X", data[i]);
                if (data[i] < 0x20 || data[i] > 0x7e)
                    *buf++ = '.';
                else
                    *buf++ = data[i];
            } else {
                PRINT_HEX_PRINTF("   ");
                *buf++ = ' ';
            }

            if (i == 7) {
                PRINT_HEX_PRINTF(" ");
                *buf++ = ' ';
            }
        }

        *buf++ = '\0';
        PRINT_HEX_PRINTF("  |%s|\r\n", tmpBuf);

        offset += lineLen;
        dataLen -= lineLen;
        data += lineLen;

        if (lineLen < 16 || dataLen == 0) {
            PRINT_HEX_PRINTF("DEC:%05d HEX:%04X\r\n\r\n", len, len);
            return;
        }
    }
}

// md5sum: 7EC00100007D02800100054854004758
void print_buff(char *title, unsigned char *data, int dataLen)
{
    int i;

    if (title != NULL) {
        PRINT_HEX_PRINTF("%s ", title);
    }

    for (i = 0; i < dataLen; i++) {
        PRINT_HEX_PRINTF("%02X", data[i]);
    }

    PRINT_HEX_PRINTF("\r\n\r\n");
}
