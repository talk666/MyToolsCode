#include <stdint.h>
#include <stdio.h>
#include <string.h>

//extern void debug_printf(const char *fmt, ...);
#define HEX_DUMP_PRINTF(...) printf(__VA_ARGS__)

// [ec20 send]
// 0000: 7E 25 02 00 00 7D 02 02  00 00 2B 48 54 00 47 56  |~%...}.. ..+HT.GV|
// 0010: 35 55 5C 00 00 00 00 00  00 00 00 00 00 00 00 00  |5U\..... ........|
// 0020: 00 00 00 00 00 00 00 00  00 20 01 14 16 31 00 E1  |........ . ...1..|
// 0030: 0D 02 00 0F 0A 00 F0 58  53 15 0C DA BD 05 61 7D  |.......X S.....a}|
// 0040: 02 E8 5A 7F                                       |..Z.             |
// DEC:00068 HEX:0044
void hex_dump(char *title, uint32_t offset, uint8_t *data, uint16_t data_len)
{
    int i = 0, j;
    char sbuf[25];
    uint8_t *buff = data;
    uint16_t line_len;
    uint16_t len = data_len;

    if (title != NULL) {
        HEX_DUMP_PRINTF("%s", title);
    }

    while (1) {
        j = 0;

        line_len = (len >= 16 ? 16 : len);
        HEX_DUMP_PRINTF("%06X:", offset);
        for (i = 0; i < 16; i++) {
            if (i < line_len) {
                HEX_DUMP_PRINTF(" %02X", buff[i]);
                if (buff[i] < 0x20 || buff[i] > 0x7e)
                    sbuf[j++] = '.';
                else
                    sbuf[j++] = buff[i];
            } else {
                HEX_DUMP_PRINTF("   ");
                sbuf[j++] = ' ';
            }

            if (i == 7) {
                HEX_DUMP_PRINTF(" ");
                sbuf[j++] = ' ';
            }
        }
        sbuf[j++] = '\0';

        HEX_DUMP_PRINTF("  |%s|\r\n", sbuf);

        offset += line_len;
        len -= line_len;
        buff += line_len;

        if (line_len < 16 || len == 0) {
            HEX_DUMP_PRINTF("DEC:%05d HEX:%04X\r\n\r\n", data_len, data_len);
            return;
        }
    }
}

// md5sum: 7EC00100007D02800100054854004758
void hex_dump_buff(char *title, uint8_t *data, uint16_t data_len)
{
    uint16_t i;

    if (title != NULL) {
        HEX_DUMP_PRINTF("%s ", title);
    }

    for (i = 0; i < data_len; i++) {
        HEX_DUMP_PRINTF("%02X", data[i]);
    }

    HEX_DUMP_PRINTF("\r\n\r\n");
}
