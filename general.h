#include <stdint.h>

/* just some convenient aliases */
typedef uint8_t byte;
typedef uintmax_t word;
#define WORD_BYTES sizeof(word)
#define WORD_BITS (WORD_BYTES * 8)

#include <stdio.h>

#define WHINE(...) fprintf(stderr, ##__VA_ARGS__)

