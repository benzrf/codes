#include "general.h"

typedef struct sparse {
    byte ix;
    void *item;
    struct sparse *next;
} sparse;

sparse *sparse_new(void);
void sparse_free(void (*free_item)(), sparse *s);
void **sparse_at(sparse *s, byte ix);

