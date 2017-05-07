#include "byte_io.h"

/* This is mostly equivalent to read(), except that it is
 * guaranteed to read as many bytes as the file descriptor
 * will provide before EOF or an error.
 * This is useful when we have a fixed word size that's
 * larger than a byte, since using read() directly might
 * give us less than we need.
 * Based on this similar function given to me by a person
 * in ##C on Freenode: http://ix.io/rUp/c
 */
ssize_t read_amap(int fd, void *buf, size_t count) {
    /* The implementation is simple: Just keep calling read()
     * until it returns 0 or the total number of bytes read
     * reaches the requested amount. */
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

/* These are just some meaningful aliases. */

ssize_t write_byte(int fd, byte x) {
    return write(fd, &x, 1);
}

ssize_t write_word(int fd, word x) {
    return write(fd, &x, WORD_BYTES);
}

