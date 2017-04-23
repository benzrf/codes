#include "general.h"

/* A struct for holding the current state in the process of
 * reading bits from some file descriptor.
 */
typedef struct bits_in {
    int in;
    word buffer;
    byte buffer_length;
} bits_in;
#define BITS_IN(in) ((bits_in) {(in), 0, 0});

/* A struct for holding the current state in the process of
 * writing bits to some file descriptor.
 *
 * Invariant: buffer_length should always be < WORD_BITS.
 */
typedef struct bits_out {
    int out;
    word buffer;
    byte buffer_length;
} bits_out;
#define BITS_OUT(out) ((bits_out) {(out), 0, 0});

signed char read_bit(bits_in *bi);

void write_bits(bits_out *bo, byte bit_count, word bits);
byte flush_bits(bits_out *bo);

