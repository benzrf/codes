#include <stdlib.h>

#include "sparse.h"

/* Allocate a new sparse.
 * It'll have a single node at index 0, initialized
 * to hold a null pointer.
 */
sparse *sparse_new(void) {
    sparse *s = malloc(sizeof(sparse));
    s->ix = 0;
    s->item = NULL;
    s->next = NULL;
    return s;
}

/* Given a function to free elements, free an entire
 * sparse.
 */
void sparse_free(void (*free_item)(), sparse *s) {
    sparse *prev = s;
    while (s != NULL) {
        if (s->item != NULL) free_item(s->item);
        prev = s;
        s = s->next;
        free(prev);
    }
}

/* Index into a sparse. Returns a pointer to the node's
 * item field, *not* the field's value! So this can be
 * used for updates too.
 * If there's no node with that index, insert a new one
 * at the correct spot, initialized with a null pointer,
 * then return a pointer to the new node's item field.
 * (Hmm... Now that I think about it, I'm not sure that
 * keeping the nodes ordered actually makes any difference.
 * Maybe I should be using trees. Whatever, this is nice
 * and simple.)
 */
void **sparse_at(sparse *s, byte ix) {
    sparse *prev = s;
    /* This loop exits once either:
     * 1. s is the node we wanted.
     * 2. The node we wanted doesn't exist, and s
     *    is now the one after where it should be. */
    while (s != NULL && s->ix < ix) {
        prev = s;
        s = s->next;
    }
    /* if the loop exited because of (2), insert the new node */
    if (s == NULL || s->ix > ix) {
        sparse *next = s;
        s = malloc(sizeof(sparse));
        s->ix = ix;
        s->item = NULL;
        s->next = next;
        prev->next = s;
    }
    return &s->item;
}

