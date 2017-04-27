#include <unistd.h>
#include <stdlib.h>

#include "hamming.h"

typedef uint16_t byte2;

/* the [COL]umns of the [E]ncoding matrix */
#define ECOL0 0x0087
#define ECOL1 0x0099
#define ECOL2 0x00AA
#define ECOL3 0x004B
#define ECOL4 0x8700
#define ECOL5 0x9900
#define ECOL6 0xAA00
#define ECOL7 0x4B00
/* E(x, n) is the nth bit of x times the nth column of the
 * encoding matrix. */
#define E(x, n) ((x & (1 << n)) ? ECOL##n : 0)
/* so applying the matrix is just summing E(x, 0..7) */
#define HAMMING_ENCODE_BYTE(x)\
    E(x, 0) ^ E(x, 1) ^ E(x, 2) ^ E(x, 3) ^\
    E(x, 4) ^ E(x, 5) ^ E(x, 6) ^ E(x, 7)

void hamming_encode(int in, int out) {
    byte next_byte;
    while (read(in, &next_byte, 1)) {
        byte2 encoded = HAMMING_ENCODE_BYTE(next_byte);
        write(out, &encoded, 2);
    }
}

/* the [COL]umns of the [C]heck matrix */
#define CCOL0 0x09
#define CCOL1 0x0A
#define CCOL2 0x0B
#define CCOL3 0x0C
#define CCOL4 0x0D
#define CCOL5 0x0E
#define CCOL6 0x0F
#define CCOL7 0x08
#define CCOL8 0x90
#define CCOL9 0xA0
#define CCOLA 0xB0
#define CCOLB 0xC0
#define CCOLC 0xD0
#define CCOLD 0xE0
#define CCOLE 0xF0
#define CCOLF 0x80
/* C(x, n) is the nth bit of x times the nth column of the
 * check matrix. */
#define C(x, n) ((x & (1 << 0x##n)) ? CCOL##n : 0)
/* so applying the matrix is just summing C(x, 0..F) */
#define HAMMING_CHECK_BYTE2(x)\
    C(x, 0) ^ C(x, 1) ^ C(x, 2) ^ C(x, 3) ^\
    C(x, 4) ^ C(x, 5) ^ C(x, 6) ^ C(x, 7) ^\
    C(x, 8) ^ C(x, 9) ^ C(x, A) ^ C(x, B) ^\
    C(x, C) ^ C(x, D) ^ C(x, E) ^ C(x, F)
/* P(x, s, d) is the s'th bit of x in place d */
#define P(x, s, d) ((x & (1 << 0x##s)) ? (1 << d) : 0)
#define HAMMING_PROJECT(x)\
    P(x, 2, 0) | P(x, 4, 1) | P(x, 5, 2) | P(x, 6, 3) |\
    P(x, A, 4) | P(x, C, 5) | P(x, D, 6) | P(x, E, 7)

void hamming_decode(int in, int out) {
    byte2 next_byte2;
    /* TODO: handle the case where it's only 1 byte */
    while (read(in, &next_byte2, 2)) {
        byte check = HAMMING_CHECK_BYTE2(next_byte2),
             check_lo = check & 15,
             check_hi = check >> 4;
        if (check_lo & 8) {
            check_lo = (check_lo - 1) & 7;
            next_byte2 ^= 1 << check_lo;
        }
        else if (check_lo) {
            /* TODO: deal with double error */
            exit(4);
        }
        if (check_hi & 8) {
            check_hi = (check_hi - 1) & 7;
            next_byte2 ^= 1 << (check_hi + 8);
        }
        else if (check_hi) {
            /* TODO: deal with double error */
            exit(4);
        }
        byte decoded = HAMMING_PROJECT(next_byte2);
        write(out, &decoded, 1);
    }
}

