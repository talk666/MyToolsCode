#ifndef __HEX_DUMP_H__
#define __HEX_DUMP_H__

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
extern void hex_dump(char *title, uint32_t offset, uint8_t *data, uint16_t data_len);

/**
 * @brief 用于打印 md5 sha256 等密钥
 * @note
 * @param  *title:
 * @param  *data:
 * @param  data_len:
 * @retval None
 */
extern void hex_dump_buff(char *title, uint8_t *data, uint16_t data_len);

#endif
