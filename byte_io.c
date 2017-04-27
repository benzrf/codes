#include "byte_io.h"

ssize_t read_amap(int fd, void *buf, size_t count) {
    byte *buf_ = buf;
    ssize_t remaining = count;
    while (remaining > 0) {
        ssize_t nread = read(fd, buf_, remaining);
        if (!nread) break;
        buf_ += nread;
        remaining -= nread;
    }
    return count - remaining;
}

ssize_t write_byte(int fd, byte x) {
    return write(fd, &x, 1);
}

ssize_t write_word(int fd, word x) {
    return write(fd, &x, WORD_BYTES);
}

