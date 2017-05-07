#include <stdlib.h>

#include "byte_io.h"
#include "bit_io.h"
#include "lzw_decode.h"

typedef struct data_word {
    int prev;
    byte last;
    short length;
} data_word;

/* Returns the first byte of the data word.
 */
byte write_data_word(data_word *dict, int ix, int out) {
    short buf_ix = dict[ix].length - 1;
    byte buf[buf_ix + 1];
    while (ix != -1) {
        buf[buf_ix--] = dict[ix].last;
        ix = dict[ix].prev;
    }
    write(out, buf, sizeof(buf));
    return buf[0];
}

void lzw_decode(int in, int out) {
    bits_in bi = BITS_IN(in);

    word next_ix;
    if (read_bits(&bi, 8, &next_ix) != 8) return;
    write_byte(out, next_ix);

    word max_ix = 256, next_power = 512;
    byte bit_count = 9;
    data_word *dict = malloc(sizeof(data_word) * next_power);
    for (int i = 0; i < 256; i++) {
        dict[i] = (data_word){-1, i, 1};
    }
    int prev = next_ix;

    int invalid = 0;
    while (read_bits(&bi, bit_count, &next_ix) == bit_count) {
        byte first;
        if (next_ix < max_ix) {
            first = write_data_word(dict, next_ix, out);
        }
        else if (next_ix == max_ix) {
            first = write_data_word(dict, prev, out);
            write_byte(out, first);
        }
        else {
            WHINE("lzw_decode: invalid index %lu\n", next_ix);
            invalid++;
            if (invalid >= 10) {
                WHINE("exiting after 10 invalid indices; "
                        "input is probably corrupt\n");
                exit(3);
            }
            first = 0;
        }
        max_ix++;
        if (max_ix >= next_power) {
            next_power <<= 1;
            bit_count++;
            /* TODO: consider the edge case where this unnecessarily
             * allocates a bunch of extra memory when the input
             * length is a power of two */
            dict = realloc(dict, sizeof(data_word) * next_power);
        }
        dict[max_ix - 1] = (data_word){prev, first, dict[prev].length + 1};
        prev = next_ix;
    }

    free(dict);
}

