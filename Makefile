CC=gcc
CF=-g -Wall -Wextra

build: tema4.o huffman.o queue.o heap.o
	$(CC) tema4.o huffman.o queue.o heap.o -o hufftar

tema4.o: tema4.c huffman.h
	$(CC) $(CF) -c tema4.c
huffman.o: huffman.c huffman.h queue.h heap.h
	$(CC) $(CF) -c huffman.c
queue.o: queue.c queue.h
	$(CC) $(CF) -c queue.c
heap.o: heap.c heap.h
	$(CC) $(CF) -c heap.c

clean:
	rm -rf *.o hufftar

.PHONY: build
.PHONY: clean
