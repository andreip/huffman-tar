#ifndef huffman_h
#define huffman_h

#include <stdint.h>

#define PRINT_USAGE fprintf(stderr, "Usage <exec> <option [compress][extract][list]> <file.huff> [folder_out] [list_files]\n")

typedef struct _ArchiveHeader {
	int16_t huffman_nodes_n;
	uint16_t huffman_root_node;
	int16_t file_entries_n;
} __attribute__((__packed__)) ArchiveHeader;

typedef struct _HuffmanNode {
	unsigned char value;
	int16_t left, right;
} __attribute__((__packed__)) HuffmanNode;

#define MAX_FILENAME_SIZE 30

typedef struct _FileEntryHeader {
	char file_name[MAX_FILENAME_SIZE];
	int32_t file_size;
	int32_t bytes_n;
	uint32_t checksum;
} __attribute__((__packed__)) FileEntryHeader;

typedef struct _HuffmanArchive {
	ArchiveHeader ah;
	HuffmanNode nodes[511];
	FileEntryHeader *fh;
} HuffmanArchive;

void HuffmanCompress(int argc, char *argv[]);
void HuffmanList(int argc, char *argv[]);
void HuffmanExtract(int argc, char *argv[]);

#endif
