#include <unistd.h>
#include <stdio.h>

#include "bit_io.h"

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

void write_bits(bits_out *bo, byte bit_count, word bits) {
    byte bits_left = WORD_BITS - bo->buffer_length;
    if (bits_left <= bit_count) {
        word bytes = bo->buffer | (bits << bo->buffer_length);
        write(bo->out, &bytes, WORD_BYTES);
        bo->buffer = bits >> bits_left;
        bo->buffer_length = bit_count - bits_left;
    }
    else {
        bo->buffer |= bits << bo->buffer_length;
        bo->buffer_length += bit_count;
    }
}

/* return value: number of excess zero bits in the final byte. */
byte flush_bits(bits_out *bo) {
    byte bl = bo->buffer_length,
         buffered = bl / 8 + (bl % 8 != 0);
    write(bo->out, &bo->buffer, buffered);
    return 8 * buffered - bl;
}


void bits_out_example(void) {
    bits_out bo = BITS_OUT(STDOUT_FILENO);
    for (word i = 0; i < 127; i++) {
        write_bits(&bo, 7, i);
    }
    int leftover = flush_bits(&bo);
    fprintf(stderr, "%d\n", leftover);
}

