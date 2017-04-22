#include <stdlib.h>

#include "sparse.h"

sparse *sparse_new(void) {
    sparse *s = malloc(sizeof(sparse));
    s->ix = 0;
    s->item = NULL;
    s->next = NULL;
    return s;
}

void sparse_free(void (*free_item)(), sparse *s) {
    while (s != NULL) {
        if (s->item != NULL) free_item(s->item);
        sparse *next = s->next;
        free(s);
        s = next;
    }
}

void **sparse_at(sparse *s, byte ix) {
    sparse *prev = s;
    while (s != NULL && s->ix < ix) {
        prev = s;
        s = s->next;
    }
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

