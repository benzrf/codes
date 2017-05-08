#include <stdlib.h>

#include "byte_io.h"
#include "hamming.h"

typedef uint16_t byte2;

/* We hardcode the matrices for Hamming(8, 4) for speed.
 * Since our numbers are little-endian as bit-vectors,
 * the literals look backwards. */

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

/* Perform Hamming(8, 4) encoding, reading from file descriptor
 * in and writing to file descriptor out.
 * Writes two bytes for each input byte.
 */
void hamming_encode(int in, int out) {
    /* This is really simple! */
    byte next_byte;
    while (read_byte(in, &next_byte)) {
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
/* P(x, s, d)'s d'th bit is the s'th bit of x */
#define P(x, s, d) ((x & (1 << 0x##s)) ? (1 << d) : 0)
/* HAMMING_PROJECT(x), where x is a byte2, is the byte of
 * data you get by throwing away the check bits. */
#define HAMMING_PROJECT(x)\
    P(x, 2, 0) | P(x, 4, 1) | P(x, 5, 2) | P(x, 6, 3) |\
    P(x, A, 4) | P(x, C, 5) | P(x, D, 6) | P(x, E, 7)

/* Perform Hamming(8, 4) decoding, reading from file descriptor
 * in and writing to file descriptor out.
 * Writes one bytes for each two input bytes.
 */
void hamming_decode(int in, int out) {
    byte2 next_byte2;
    /* we want to know how many bytes the read_amap() fetches
     * so that we can pad with zeroes if necessary */
    int nread;
    while ((nread = read_amap(in, &next_byte2, 2))) {
        if (nread == 1) next_byte2 &= 0xFF;
        /* check_lo is the syndrome for the lower 8 bits of
         * next_byte2; check_hi is the syndrome for the upper
         * 8 bits */
        byte check = HAMMING_CHECK_BYTE2(next_byte2),
             check_lo = check & 15,
             check_hi = check >> 4;
        /* if the high bit is set, we have an odd number of
         * errors, which we assume is just 1 */
        if (check_lo & 8) {
            /* If the error is anywhere but the extended bit,
             * the lower 3 bits of the syndrome will be its
             * 1-based index, so we need to extract the lower
             * 3 bits and subtract 1. If the error is in the
             * extended bit, the syndrome will be 0b1000.
             * (check_lo - 1) & 7 does the trick in either
             * case. */
            check_lo = (check_lo - 1) & 7;
            /* just flip the incorrect bit */
            next_byte2 ^= 1 << check_lo;
        }
        else if (check_lo) {
            /* any nonzero syndrome with a zero high bit can
             * only result from an even, nonzero number of
             * errors, which we assume is 2 */
            WHINE("hamming_decode: double error in byte %02x\n",
                    next_byte2 & 0xFF);
        }
        /* the next bit is all of the same logic but for the
         * upper 8 bits of next_byte2 */
        if (check_hi & 8) {
            check_hi = (check_hi - 1) & 7;
            next_byte2 ^= 1 << (check_hi + 8);
        }
        else if (check_hi) {
            WHINE("hamming_decode: double error in byte %02x\n",
                    next_byte2 >> 8);
        }
        /* now that we've used the parity information, we
         * throw it away and write the data bits */
        write_byte(out, HAMMING_PROJECT(next_byte2));
    }
}

