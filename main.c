#include <unistd.h>

#include "lzw_encode.h"
#include "lzw_decode.h"

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    if (argv[1][0] == 'e') lzw_encode(STDIN_FILENO, STDOUT_FILENO);
    else if (argv[1][0] == 'd') lzw_decode(STDIN_FILENO, STDOUT_FILENO);
    else return 2;
    return 0;
}

