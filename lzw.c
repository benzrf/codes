#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "general.h"
#include "sparse.h"

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

/* A 256-ary tree, with words at the nodes.
 * Used for the LZW dictionary.
 */
typedef struct bytetree {
    word ix;
    sparse *children;
} bytetree;
#define NEW_BYTETREE(var, ixv) {\
    var = malloc(sizeof(bytetree));\
    (var)->ix = (ixv);\
    (var)->children = sparse_new();\
}


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

void bytetree_free(bytetree *tree) {
    sparse_free(bytetree_free, tree->children);
    free(tree);
}


void lzw_encode(int in, int out) {
    byte next_byte;
    if (!(next_byte = read(in, &next_byte, 1))) return;

    bits_out bo = BITS_OUT(out);

    word maxix = 255, next_power = 256;
    byte bit_count = 8;
    bytetree *dict_root[256];
    for (int i = 0; i < 256; i++) {
        NEW_BYTETREE(dict_root[i], i);
    }
    bytetree *dict_cur = dict_root[next_byte];

    while (read(in, &next_byte, 1)) {
        bytetree **next =
            (bytetree**)sparse_at(dict_cur->children, next_byte);
        if (*next != NULL) dict_cur = *next;
        else {
            write_bits(&bo, bit_count, dict_cur->ix);
            maxix++;
            if (maxix == next_power) {
                next_power <<= 1;
                bit_count++;
            }
            NEW_BYTETREE(*next, maxix);
            dict_cur = dict_root[next_byte];
        }
    }
    write_bits(&bo, bit_count, dict_cur->ix);
    byte excess = flush_bits(&bo);
    write(out, &excess, 1);

    for (int i = 0; i < 256; i++) {
        bytetree_free(dict_root[i]);
    }
}


int main(void) {
    lzw_encode(STDIN_FILENO, STDOUT_FILENO);
    return 0;
}

