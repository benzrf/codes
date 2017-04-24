#include "general.h"

/* A struct for holding the current state in the process of
 * reading bits from some file descriptor.
 */
typedef struct bits_in {
    /* the file descriptor */
    int in;

    /* a "buffer" of read-but-unconsumed bits */
    word buffer;

    /* number of data bits in the buffer */
    byte buffer_length;
} bits_in;
#define BITS_IN(in) ((bits_in) {(in), 0, 0});

/* A struct for holding the current state in the process of
 * writing bits to some file descriptor.
 *
 * Invariant: buffer_length should always be < WORD_BITS.
 */
typedef struct bits_out {
    /* the file descriptor */
    int out;

    /* a "buffer" of read-but-unconsumed bits.
     * we could technically buffer as little as a byte at a time,
     * but this saves on write calls. */
    word buffer;

    /* number of data bits in the buffer */
    byte buffer_length;
} bits_out;
#define BITS_OUT(out) ((bits_out) {(out), 0, 0});

byte read_bits(bits_in *bi, byte bit_count, word *out);

void write_bits(bits_out *bo, byte bit_count, word bits);
byte flush_bits(bits_out *bo);

