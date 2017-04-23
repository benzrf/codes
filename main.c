#include <unistd.h>

#include "lzw.h"

int main(void) {
    lzw_encode(STDIN_FILENO, STDOUT_FILENO);
    return 0;
}


