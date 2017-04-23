#include <unistd.h>

#include "lzw_encode.h"

int main(void) {
    lzw_encode(STDIN_FILENO, STDOUT_FILENO);
    return 0;
}

