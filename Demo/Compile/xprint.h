#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdint.h>

/**
 * @brief 相当于 hexdump -C file.txt
 * @note
 * @param  *title:
 * @param  offset:
 * @param  *data:
 * @param  data_len:
 * @retval None
 */
extern void xprint_hex(char *title, int offset, unsigned char *data, int dataLen);

/**
 * @brief 用于打印 md5 sha256 等密钥
 * @note
 * @param  *title:
 * @param  *data:
 * @param  data_len:
 * @retval None
 */
extern void xprint_buff(char *title, unsigned char *data, int dataLen);

#endif
