#include <unistd.h>
#include <stdlib.h>

#include "bit_io.h"
#include "sparse.h"
#include "lzw_encode.h"

/* A 256-ary tree, with words at the nodes.
 * Used for the LZW dictionary.
 */
typedef struct bytetree {
    /* the code number for the data word described
     * by this node's position in the tree */
    word ix;

    sparse *children;
} bytetree;

/* Allocate a bytetree node and a sparse for its children.
 */
static bytetree *bytetree_new(word ix) {
    bytetree *tree = malloc(sizeof(bytetree));
    tree->ix = ix;
    tree->children = sparse_new();
    return tree;
}

/* Recursively free an entire bytetree.
 */
static void bytetree_free(bytetree *tree) {
    sparse_free(bytetree_free, tree->children);
    free(tree);
}


/* Perform LZW encoding, reading from file descriptor in
 * and writing to file descriptor out.
 * Each byte of input is considered a symbol, but output is
 * bit-packed.
 */
void lzw_encode(int in, int out) {
    /* We'll use normal read() calls for input, but since output
     * is bit-packed, we'll use a bits_out. */
    bits_out bo = BITS_OUT(out);

    /* Our tree's root will be special-cased as an actual array,
     * but the main loop assumes a normal bytetree "current node".
     * So we need to manually take our first step before starting
     * the main loop; hence we read a byte right at the beginning. */
    byte next_byte;
    if (!read(in, &next_byte, 1)) return;

    /* max_ix is the current largest code number. bit_count
     * is the number of bits necessary to store max_ix; we
     * store it redundantly to avoid recomputing. next_power
     * is the next power of 2 after max_ix; we'll know to
     * increment bit_count when max_ix reaches next_power. */
    word max_ix = 255, next_power = 256;
    byte bit_count = 8;
    /* We special-case dict_root as an actual array since
     * we know for certain that it will be frequently traversed
     * and that it will use all 256 possible child nodes—no
     * sense wasting time and space with a linked list. Also,
     * it has no associated code number, so it doesn't even
     * need to be a bytetree node. */
    bytetree *dict_root[256];
    for (int i = 0; i < 256; i++) {
        dict_root[i] = bytetree_new(i);
    }
    /* the "current node" */
    bytetree *dict_cur = dict_root[next_byte];

    while (read(in, &next_byte, 1)) {
        bytetree **next =
            (bytetree**)sparse_at(dict_cur->children, next_byte);
        if (*next != NULL) {
            /* If we're still in a prefix of a data word that's
             * already in the dictionary, just continue down. */
            dict_cur = *next;
        }
        else {
            /* But if appending the next symbol produces an unknown
             * word, write the code number for the word we had
             * before the append... */
            write_bits(&bo, bit_count, dict_cur->ix);
            /* ...make a new node for the unknown word, assigning
             * it the next index... */
            max_ix++;
            if (max_ix >= next_power) {
                /* (start using more bits per code number if necessary) */
                next_power <<= 1;
                bit_count++;
            }
            *next = bytetree_new(max_ix);
            /* ...and then treat the symbol as the first symbol of
             * a new word. */
            dict_cur = dict_root[next_byte];
        }
    }
    /* We're at EOF, so whatever known word we're in the middle of
     * is in fact the whole word, so write its code number. */
    write_bits(&bo, bit_count, dict_cur->ix);
    /* we're done writing now, so flush any buffered bits */
    flush_bits(&bo);

    /* finally, clean up after ourselves */
    for (int i = 0; i < 256; i++) {
        bytetree_free(dict_root[i]);
    }
}

