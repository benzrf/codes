CC=gcc
CC_FLAGS=-c -Wall -Wpedantic -O2

BINARY=lzw
OBJECTS=sparse.o lzw.o

$(BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BINARY)

%.o: %.c
	$(CC) $(CC_FLAGS) $<

clean:
	rm -f $(OBJECTS) $(BINARY)

.PHONY: clean

