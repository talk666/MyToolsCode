#ifndef __PORTING_H__
#define __PORTING_H__

#include <stdint.h>

uint64_t __get_time();

int __get_entropy(uint8_t *data, uint32_t len);

#endif
