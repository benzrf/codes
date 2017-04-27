CC=gcc -Wall -Wpedantic $(if $(debug),-ggdb,-O2)

BINARY=code
OBJECTS=byte_io.o bit_io.o sparse.o lzw_encode.o lzw_decode.o hamming.o

$(BINARY): main.c $(OBJECTS)
	$(CC) main.c $(OBJECTS) -o $(BINARY)

%.o: %.c
	$(CC) -c $<

clean:
	rm -f $(OBJECTS) $(BINARY)

.PHONY: clean

