#include <unistd.h>

#include "general.h"

#define read_byte(fd, buf) read(fd, buf, 1)

/* "amap" = as much as possible */
ssize_t read_amap(int fd, void *buf, size_t count);
ssize_t write_byte(int fd, byte x);
ssize_t write_word(int fd, word x);

