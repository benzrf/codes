#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "lzw_encode.h"
#include "lzw_decode.h"
#include "hamming.h"

typedef void (*const stage)(int, int);

void pipeline(int in, int out, stage *steps) {
    for (; *(steps + 1) != NULL; steps++) {
        int fds[2];
        pipe(fds);
        if (!fork()) {
            close(fds[0]);
            (*steps)(in, fds[1]);
            close(in);
            close(fds[1]);
            return;
        }
        close(fds[1]);
        in = fds[0];
    }
    (*steps)(in, out);
    close(in);
    close(out);
}

#define SUB(c, ...) else if (!strcmp(argv[1], #c)) {\
    const stage p[] = {__VA_ARGS__, NULL};\
    pipeline(STDIN_FILENO, STDOUT_FILENO, p);\
}
#define ERR(code, msg, ...) {\
    fprintf(stderr, "%s: " msg "\n", argv[0], ##__VA_ARGS__);\
    return code;\
}

int main(int argc, char *argv[]) {
    if (argc < 2) ERR(1, "no subcommand given")
    SUB(compress,   lzw_encode)
    SUB(decompress, lzw_decode)
    SUB(lzw_id,     lzw_encode, lzw_decode)
    SUB(augment,    hamming_encode)
    SUB(correct,    hamming_decode)
    SUB(hamming_id, hamming_encode, hamming_decode)
    SUB(encode,     lzw_encode, hamming_encode)
    SUB(decode,     hamming_decode, lzw_decode)
    SUB(full_id,    lzw_encode, hamming_encode, hamming_decode, lzw_decode)
    else ERR(2, "unknown subcommand %s", argv[1])
    return 0;
}

