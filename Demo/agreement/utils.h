#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BETWEEN
    #define BETWEEN(VAL, vmin, vmax) ((VAL) >= (vmin) && (VAL) <= (vmax))
#endif

#ifndef IS_HEX_CHAR
    #define IS_HEX_CHAR(VAL) (BETWEEN(VAL, '0', '9') || BETWEEN(VAL, 'A', 'F') || BETWEEN(VAL, 'a', 'f'))
#endif

    extern int hex_string_to_array(const char *inStr, unsigned char *outArray, unsigned int *outArrayLen);

#ifdef __cplusplus
}
#endif

#endif