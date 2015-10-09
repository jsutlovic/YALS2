#include "base64.h"

const char *b64trans = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * Get the index value of a character in the Base-64 translation table
 */
static int _char_ind(const char l) {
    if (l >= 'A' && l <= 'Z') {
        return l - 'A';
    } else if (l >= 'a' && l <= 'z') {
        return 26 + l - 'a';
    } else if (l >= '0' && l <= '9') {
        return 52 + l - '0';
    } else if (l == '+') {
        return 62;
    } else if (l == '/') {
        return 63;
    } else {
        return -1;
    }
}

/*
 * Encode a segment of 3 chars into a Base64-encoded segment of 4 chars
 */
void _b64seg_enc(const char *seg, char *b64seg, size_t to_end) {
    uint32_t seg_temp = 0;
    uint32_t seg_mask = 0x3f; // 6 bits
    int ended = 0;

    // Shift all characters into a single bitfield
    for (size_t i = 0; i < SEG_IN_LEN; i++) {
        if (i >= to_end) {
            seg_temp = seg_temp << 8 * (SEG_IN_LEN - i);
            ended = 1;
            break;
        }
        seg_temp = (seg_temp << 8) | (uint8_t) seg[i];
    }

    // Take bitfield and split it into 6-bit chunks
    for (int i = SEG_OUT_LEN-1, j=0; i >= 0; i--, j++) {
        uint8_t index = (uint8_t) ((seg_temp >> 6 * i) & seg_mask);
        b64seg[j] = (ended && j>1 && !index) ? '=' : b64trans[index];
    }
}

/*
 * Decode a segment of 4 Base64-encoded chars into a segment of 3 chars
 */
void _b64seg_dec(const char *b64, char *plain_seg) {
    uint32_t seg_temp = 0;
    uint32_t seg_mask = 0xff; // 8 bits

    for (int i = 0; i < SEG_OUT_LEN; i++) {
        if (b64[i] == '=') {
            seg_temp <<= 6 * (SEG_OUT_LEN - i);
            break;
        }
        seg_temp = (seg_temp << 6) | ((uint8_t) _char_ind(b64[i]) & 0x3f);
    }

    for (int i = SEG_IN_LEN-1, j=0; i >= 0; i--, j++) {
        plain_seg[j] = (seg_temp >> 8*i) & seg_mask;
    }
}

/*
 * Base64-encode a bytestring
 * e.g. {0x66, 0x72, 0x6f, 0x67} -> "ZnJvZw=="
 * bytes: The char array of bytes to encode
 * in_len: The length of the characters to encode (excluding terminating \0)
 * out_len: Will be populated with the length of the encoded result
 * returns: base64-encoded char string (with terminating \0)
 */
char *b64_enc(const char *bytes, size_t in_len, size_t *out_len) {
    // base64 length is 4/3 of the decoded hex string
    size_t base64len = (size_t) (in_len / 3.0 + .7) * 4;
    char *b64enc = calloc(base64len + 1, sizeof(char));

    for (size_t i = 0, j = 0; i < in_len; i+=3, j+=4) {
        _b64seg_enc(&bytes[i], &b64enc[j], in_len - i);
    }

    if (out_len != NULL) {
        *out_len = base64len;
    }
    return b64enc;
}

/*
 * Base64-decode a bytestring
 * e.g. "ZnJvZw==" -> {0x66, 0x72, 0x6f, 0x67}
 * b64_bytes: The encoded char array to decode (must be padded)
 * in_len: The length of the characters to decode (must be divisible by 4)
 * out_len: Will be populated with the length of the decoded string
 * returns: Decoded char string (with a terminating \0)
 */
char *b64_dec(const char *b64_bytes, size_t in_len, size_t *out_len) {
    if (in_len % 4 != 0) {
        if (out_len != NULL) {
            *out_len = 0;
        }
        return NULL;
    }
    size_t plain_len = (size_t) (in_len / 4.0 + .75) * 3;
    for (size_t i = in_len - 1; i >= in_len - 2; i--) {
        if (b64_bytes[i] == '=') {
            plain_len--;
        }
    }

    char *plain = calloc(plain_len + 1, sizeof(char));

    for (size_t i = 0, j = 0; j < plain_len; i+=4, j+=3) {
        _b64seg_dec(&b64_bytes[i], &plain[j]);
    }

    if (out_len != NULL) {
        *out_len = plain_len;
    }
    return plain;
}
