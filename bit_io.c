#include <stdio.h>

#include "byte_io.h"
#include "bit_io.h"

/* Get some bits from the file descriptor and write them into
 * the bits argument, zeroing the rest of it.
 * Returns the number of input bits placed into the argument.
 */
byte read_bits(bits_in *bi, byte bit_count, word *bits) {
    /* We're going to branch, and in each branch, we will record
     * how many bits we set. Then, afterward, we'll clear any
     * unset bits. So it's OK to just copy the whole buffer to
     * begin with. */
    *bits = bi->buffer;
    /* in most cases, the number of bits we set will be the number
     * of bits requested, so we'll assume that to begin with, and
     * change it if necessary */
    byte bits_set = bit_count;
    if (bi->buffer_length >= bit_count) {
        /* if the buffer already has as many bits as we want,
         * all we need to do for this branch is remove them
         * from the buffer */
        bi->buffer >>= bit_count;
        bi->buffer_length -= bit_count;
    }
    else {
        /* otherwise, we need to do some reading and some
         * slightly-more-advanced buffer juggling */
        word input = 0;
        byte bits_needed = bit_count - bi->buffer_length,
             /* read an entire word at a time even if we need less,
              * to save on read calls—we can just buffer the rest */
             bits_read = read_amap(bi->in, &input, WORD_BYTES) * 8;
        if (bits_read < bits_needed) {
            bits_set = bi->buffer_length + bits_read;
        }
        *bits |= input << bi->buffer_length;
        /* aforementioned buffer juggling */
        bi->buffer = input >> bits_needed;
        bi->buffer_length = bits_read - bits_needed;
    }
    /* aforementioned clearing of unset bits */
    *bits &= (1 << bits_set) - 1;
    return bits_set;
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
        write_word(bo->out, bo->buffer | (bits << bo->buffer_length));
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

void bits_in_example(void) {
    bits_in bi = BITS_IN(STDIN_FILENO);
    for (int i = 1; i < 11; i++) {
        word w;
        read_bits(&bi, i, &w);
        printf("%lu\n", w);
    }
}

