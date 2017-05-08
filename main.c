#include <unistd.h>
#include <string.h>

#include "lzw_encode.h"
#include "lzw_decode.h"
#include "hamming.h"

/* A "stage" is a given encoding or decoding function:
 * Something that takes two file descriptors and doesn't
 * return anything in particular.
 */
typedef void (*const stage)(int, int);

/* Given initial input and final output file descriptors
 * and a NULL-terminated array of stages, fork off child
 * processes running stage (except the last one, which
 * runs in the original process), connecting them with
 * pipes.
 */
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

/* We list the various subcommands in subcommands.h, as calls
 * to the SUB macro that pass the subcommand name followed
 * by the pipeline stages to run. We'll use the list twice:
 * Once to generate a help message, and once to generate the
 * code that actually runs a pipeline based on the subcommand.
 * To achieve that, we'll just include subcommands.h once in
 * each place, defining SUB differently each time. Here are
 * the two versions of SUB.
 */
#define SUB_help(c, ...) WHINE(#c ": pipeline of " #__VA_ARGS__ "\n");
#define SUB_branch(c, ...) else if (!strcmp(argv[1], #c)) {\
    const stage p[] = {__VA_ARGS__, NULL};\
    pipeline(STDIN_FILENO, STDOUT_FILENO, p);\
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        WHINE("%s: no subcommand given. Use one of:\n\n", argv[0]);
#define SUB SUB_help
#include "subcommands.h"
        return 1;
    }
#define SUB SUB_branch
#include "subcommands.h"
    else {
        WHINE("%s: unknown subcommand %s. "
                "Run without arguments for a list of subcommands. \n",
                argv[0], argv[1]);
        return 2;
    }
    return 0;
}

