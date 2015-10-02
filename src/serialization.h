#ifndef _SERIALIZATION_H
#define _SERIALIZATION_H

#include <stdint.h>
#include <stdlib.h>

static inline void _ser_uint8(char *str, size_t offset, uint8_t n) {
    str[offset] = n;
}

static inline void _ser_uint16(char *str, size_t offset, uint16_t n) {
    str[offset] = (n >> 8) & 0xff;
    str[offset + 1] = n & 0xff;
}

static inline void _ser_uint32(char *str, size_t offset, uint32_t n) {
    for (int i = 0; i < 4; ++i) {
        char l = (n >> (8 * (3 - i))) & 0xff;
        str[offset + i] = l;
    }
}

static inline void _ser_uint64(char *str, size_t offset, uint64_t n) {
    for (int i = 0; i < 8; ++i) {
        char l = (n >> (8 * (7 - i))) & 0xff;
        str[offset + i] = l;
    }
}

static inline uint16_t _dser_uint16(char *str, size_t offset) {
    uint8_t *s = (uint8_t *) str;
    return s[offset] << 8 | s[offset + 1];
}

static inline uint32_t _dser_uint32(char *str, size_t offset) {
    uint8_t *s = (uint8_t *) str;
    return s[offset] << 24 | s[offset + 1] << 16 | s[offset + 2] << 8 | s[offset + 3];
}

#endif
/* vim: set ft=c : */
