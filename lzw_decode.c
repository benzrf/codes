#include <stdlib.h>

#include "byte_io.h"
#include "bit_io.h"
#include "lzw_decode.h"

/* For decoding, we want to do lookup by index rather than
 * by word, so we'll use an array (the "dictionary" type).
 * The most straightforward thing would be to put string
 * pointers in the array, but that takes up much more space
 * than necessary—any word we insert into the dictionary
 * will already have every prefix in the dictionary, because
 * of how LZW works. So instead, each element of the array
 * contains:
 */
typedef struct data_word {
    /* the index of another entry whose word is everything
     * but the last byte of this entry's word, */
    int prev;

    /* the last byte, */
    byte last;

    /* and the length of the word. */
    short length;
} data_word;

/* So, each list is stored, in effect, as a backwards linked
 * list. This gives us O(n) memory usage in the input length
 * (measured in code-numbers, so slightly better than O(n)
 * in byte length). */

typedef data_word *dictionary;

/* Given a dictionary, an index in it, and a file descriptor,
 * write the word at that index to the file descriptor.
 * When we're decoding, we need to know the first byte of each
 * index we decode because it will be the last byte of the
 * word we want to add to our dictionary. Since we're already
 * traversing the linked list in this function, we return
 * the first byte while we're at it, even though that's not
 * strictly related to its functionality.
 */
byte write_data_word(data_word *dict, int ix, int out) {
    /* Since the length of the word is part of the struct,
     * we can pre-allocate a buffer to fill with the word.
     * We'll fill it backwards as we traverse the list, so
     * the first index we fill will be the buffer's length
     * minus one. */
    short buf_ix = dict[ix].length - 1;
    byte buf[buf_ix + 1];
    /* Standard linked list traversal, albeit with array
     * indices rather than actual pointers. We use -1 for
     * the sentinel value. */
    while (ix != -1) {
        buf[buf_ix--] = dict[ix].last;
        ix = dict[ix].prev;
    }
    /* do the actual writing now that we have the word as a
     * string, then return the first char as discussed */
    write(out, buf, sizeof(buf));
    return buf[0];
}

/* Perform LZW decoding, reading from file descriptor in and
 * writing to file descriptor out.
 */
void lzw_decode(int in, int out) {
    /* lzw_encode's output is bit-packed, so we'll use a
     * bits_in to get our input. */
    bits_in bi = BITS_IN(in);

    /* Each time we read an index, we'll only have as much
     * information as lzw_encode did when it wrote the
     * _previous_ index. Therefore, we call the "most
     * recently read index" variable "next_ix", because at
     * any given point, it will be the index that lzw_encode
     * writes "next" after being in the state we currently
     * know.
     * The first index is a special case; normally, we have
     * a previously-seen index whose word we concatenate to
     * the first byte of the next index's word to determine
     * the next word to add to the dictionary, but in this
     * case, there's obviously no previous one. So we handle
     * it separately by itself. In particular, we know that
     * the first index will be 8 bits, and it will be equal
     * to the actual byte it corresponds to (since that's
     * our starting dictionary), so we can just copy a
     * single byte from in to out. */
    word next_ix = 0;
    if (!read_byte(in, &next_ix)) return;
    write_byte(out, next_ix);

    /* max_ix, next_power, and bit_count serve similar roles
     * as in lzw_encode. In this case, they refer to the
     * maximum index we may _read_, so max_ix will generally
     * be one more than the maximum index _in the dictionary_,
     * since our dictionary is one step behind. We start one
     * higher here, since we've already read the first index.
     * Note that max_ix is the "next index" in the sense of
     * being the next index we're going to add to the
     * dictionary, but next_ix is the "next index" in the
     * sense of being the next index whose word we need to
     * print. */
    word max_ix = 256, next_power = 512;
    byte bit_count = 9;
    /* We dynamically allocate the dictionary so that we can
     * grow it if necessary, since we don't know how high the
     * indices will go. To keep it simple, we'll keep its
     * size in sync with next_power—i.e., we grow by
     * doubling size, starting with 512. */
    dictionary dict = malloc(sizeof(data_word) * next_power);
    /* initialize to our starting dictionary */
    for (int i = 0; i < 256; i++) {
        /* -1 is the sentinel value for the start/end of the
         * linked list */
        dict[i] = (data_word){-1, i, 1};
    }
    /* prev will be the previous index we read */
    int prev = next_ix;

    /* Invalid is the number of too-large indices we've seen.
     * We'll exit if we see 10 of them, because the data is
     * probably too corrupt to be interesting. That should
     * probably be a percentage thing instead, but meh,
     * that would be complicated ;) */
    int invalid = 0;
    /* We'll exit the loop once we hit EOF, which should be
     * the only time read_bits returns anything other than
     * bit_count (barring an error). The final thing we read
     * won't be an index (it's too short), just garbage at
     * the end (probably zeros, for output from lzw_encode),
     * so we don't have to do anything with it. */
    while (read_bits(&bi, bit_count, &next_ix) == bit_count) {
        /* we'll set this to the first byte of next_ix's word */
        byte first;
        if (next_ix < max_ix) {
            /* if next_ix is in the dictionary, this is super
             * simple—just write that word (and snag its
             * first byte) */
            first = write_data_word(dict, next_ix, out);
        }
        else if (next_ix == max_ix) {
            /* If it's the next index we'll add, we can still
             * deal with it. The next index we add will
             * consist of the last index's word followed by
             * the next index's word's first byte, but then
             * the next index's word's first byte is the last
             * index's word's first byte, so we have everything
             * we need. */
            first = write_data_word(dict, prev, out);
            write_byte(out, first);
        }
        else {
            /* If it's larger than the next index we'll add,
             * it couldn't even have been generated by
             * lzw_encode, so we whine about it. If this is
             * the 10th time it's happened, we just exit. */
            WHINE("lzw_decode: invalid index %lu\n", next_ix);
            invalid++;
            if (invalid >= 10) {
                WHINE("lzw_decode: exiting after 10 invalid indices; "
                        "input is probably corrupt\n");
                exit(3);
            }
            /* just use 0, since there's no particular byte
             * to favor */
            first = 0;
        }
        max_ix++;
        if (max_ix >= next_power) {
            /* If the new upper bound on indices we may see
             * is too large to fit in our number of bits (and
             * additionally, therefore, out of bounds in our
             * dictionary), increase our bit count (and make
             * our dictionary bigger). */
            next_power <<= 1;
            bit_count++;
            /* There is technically an edge case where we're
             * actually on the last character and this
             * unnecessarily allocates a bunch of extra memory
             * (when the input length is a power of two), but
             * any other doubling-based approach would also
             * have similar problems, and I want to keep it
             * simple. */
            dict = realloc(dict, sizeof(data_word) * next_power);
        }
        /* Add the next index to the dictionary by consing the
         * first character of the most recent input index's word
         * to the end of the previous input index's word. Then
         * the length is just the previous input index's word's
         * length plus one. */
        dict[max_ix - 1] = (data_word){prev, first, dict[prev].length + 1};
        /* Then remember our most recent input index as the
         * previous one, because we're about to read another
         * index. */
        prev = next_ix;
    }

    /* Don't leak! */
    free(dict);
}

