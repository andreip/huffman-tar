CC=gcc
CF=-g -Wall -Wextra

build: main.o huffman.o queue.o heap.o
	$(CC) main.o huffman.o queue.o heap.o -o hufftar

main.o: main.c huffman.h
	$(CC) $(CF) -c main.c
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
