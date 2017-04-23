#include <unistd.h>
#include <stdlib.h>

#include "bit_io.h"
#include "sparse.h"
#include "lzw.h"

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

static void bytetree_free(bytetree *tree) {
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

