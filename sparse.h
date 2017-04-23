#include "general.h"

/* A "sparse array" (not sure what the correct name for this is).
 * It's a linked list whose nodes are tagged with indices; using
 * a non-existent index will insert it in the correct place.
 * Always start with at least a 0 node at the beginning.
 *
 * I'm using this instead of just a 256-element array for space
 * savings; most nodes will only have several children, so this
 * uses up to 10x less memory. Indexing is O(n)-ish instead of O(1),
 * but it will usually be fast since, again, most nodes will only
 * have several children.
 */
typedef struct sparse {
    byte ix;
    void *item;
    struct sparse *next;
} sparse;

sparse *sparse_new(void);
void sparse_free(void (*free_item)(), sparse *s);
void **sparse_at(sparse *s, byte ix);

