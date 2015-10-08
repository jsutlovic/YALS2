#ifndef _BASE64_H
#define _BASE64_H
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SEG_IN_LEN 3
#define SEG_OUT_LEN 4

char *b64_enc(const char *bytes, size_t in_len, size_t *out_len);
char *b64_dec(const char *b64_bytes, size_t in_len, size_t *out_len);

#endif
/* vim: set ft=c : */
