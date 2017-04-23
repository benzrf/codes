#include <unistd.h>
#include <stdio.h>

#include "bit_io.h"

/* Get the next bit from the file descriptor.
 * Returns 0 or 1 normally, or -1 at EOF.
 */
signed char read_bit(bits_in *bi) {
    if (bi->buffer_length == 0) {
        ssize_t nread = read(bi->in, &bi->buffer, WORD_BYTES);
        if (nread == 0) return -1;
        bi->buffer_length = nread * 8;
    }
    byte bit = bi->buffer & 1;
    bi->buffer >>= 1;
    bi->buffer_length--;
    return bit;
}

/* Send up to WORD_BITS bits to the file descriptor.
 * Probably won't immediately write all of them,
 * unless the word boundaries line up.
 */
void write_bits(bits_out *bo, byte bit_count, word bits) {
    byte bits_left = WORD_BITS - bo->buffer_length;
    if (bits_left <= bit_count) {
        /* if we're writing enough bits to fill our buffer,
         * write the filled buffer and replace the buffer with
         * the excess bits (if any) */
        word bytes = bo->buffer | (bits << bo->buffer_length);
        write(bo->out, &bytes, WORD_BYTES);
        bo->buffer = bits >> bits_left;
        bo->buffer_length = bit_count - bits_left;
    }
    else {
        /* otherwise, just buffer all of the bits */
        bo->buffer |= bits << bo->buffer_length;
        bo->buffer_length += bit_count;
    }
}

/* Send any remaining buffered bits to the file descriptor,
 * padding with zeros if necessary. Only call this once, when
 * done with the bits_out.
 * Returns the number of excess zeros used for padding.
 */
byte flush_bits(bits_out *bo) {
    byte bl = bo->buffer_length,
         buffered = bl / 8 + (bl % 8 != 0);
    write(bo->out, &bo->buffer, buffered);
    return 8 * buffered - bl;
}


/* Just a simple example of the usage of bits_out.
 * Pipe stdout into the following Ruby one-liner to see
 * some counting in little-endian binary:
 * puts $<.each_byte.map{|b|format('%08b',b).reverse}.join.scan /.{,7}/
 */
void bits_out_example(void) {
    bits_out bo = BITS_OUT(STDOUT_FILENO);
    for (word i = 0; i < 127; i++) {
        write_bits(&bo, 7, i);
    }
    int leftover = flush_bits(&bo);
    fprintf(stderr, "%d\n", leftover);
}

