#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "queue.h"
#include "heap.h"

#define HAS_CHILDREN(node)      (node->left || node->right)

typedef struct _node {
	int repeat;                 //use as probability
	unsigned char value;
	struct _node *left, *right;
} Node;

static int CompareByRepeat(void *a, void *b) {
	Node *x = a;
	Node *y = b;
	return x->repeat - y->repeat;
}

static int FileGetSize(FILE *f) {
	int initial = ftell(f);
	int pos;

	fseek(f, 0, SEEK_END);
	pos = ftell(f);
	fseek(f, initial, SEEK_SET);
	return pos;
}

static void FreeCodes(char *codes[]) {
	int i;

	for(i=0; i<256; i++)
		if(codes[i])
			free(codes[i]);
}

static int MyStrcmp(const void *a, const void *b) {
	const FileEntryHeader *x = a;
	const FileEntryHeader *y = b;
	return strcmp(x->file_name, y->file_name);
}

/* path/to/file returns file */
static char* TruncateName(char *file) {
	char *c;

	c = strrchr(file, '/');
	if (c)
		return c+1;
	return file;
}

/* Open files to compress and deal with errors.
 * Also, save details of the archive, to be 
 * written to file.
 */
static void OpenFiles(FILE **f, HuffmanArchive *HA, int argc, char *argv[]) {
	int i, j, files_n;
	char *tmp;
	
	HA->fh = NULL;
	HA->ah.file_entries_n = 0;
	files_n = HA->ah.file_entries_n;

	for(i=0; i<argc-3; i++) {
		f[i] = fopen(argv[3+i], "rb");
		if (f[i] == NULL) {
			fprintf(stderr, "Error: %s can't be opened\n", argv[3+i]);
			continue;
		}
		tmp = TruncateName(argv[3+i]);

		/* verify if it's a duplicate */
		for(j=0; j<files_n; j++)
			if (!strcmp (tmp, HA->fh[j].file_name))
				break;
		if (j != files_n) {
			fclose(f[i]);
			f[i] = NULL;
			continue;
		}

		/* make room for this new file, which is not a duplicate */
		HA->fh = realloc (HA->fh, (files_n+1) * sizeof(FileEntryHeader));
		memset(&HA->fh[files_n].file_name, 0, MAX_FILENAME_SIZE);
		memcpy(HA->fh[files_n].file_name, tmp, strlen(tmp)+1);

		files_n = ++HA->ah.file_entries_n;
	}
	if (!files_n) {
		fprintf(stderr, "Error: No input files\n");
		exit(1);
	}
}

/* This function adds codes to a table as strings;
 * if going on left, concatenate 0, if right, 1
 * if it's a leaf, add the code obtained
 */
static void ObtainCodifications(Node *root, char *code, char **codes) {
	if (root == NULL)
		return;

	if (root->left) {
		strcat(code, "0");
		ObtainCodifications (root->left, code, codes);
		code[strlen(code)-1] = '\0';
	}
	if (root->right) {
		strcat(code, "1");
		ObtainCodifications (root->right, code, codes);
		code[strlen(code)-1] = '\0';
	}
	if ( !HAS_CHILDREN(root) ) {
		codes[root->value] = strdup(code);
		return;
	}
}

/* This function updates the HuffmanNode vector;
 * it uses a queue to represent the tree in order
 * to be writen to file
 */
static void UpdateNodes(Node *root, HuffmanNode *nodes) {
	Queue q = QueueCreate();
	Node *node = NULL;
	int n = 0;

	if (root == NULL)
		return;
	
	Enqueue(q, root);
	while (!QueueEmpty(q)) {
		node = Dequeue(q);
		
		if (node->left) {
			Enqueue(q, node->left);
			nodes[n].left = QueueSize(q) + n;
		} else
			nodes[n].left = -1;

		if (node->right) {
			Enqueue(q, node->right);
			nodes[n].right = QueueSize(q) + n;
		} else
			nodes[n].right = -1;

		if ( !HAS_CHILDREN(node) ) {
			nodes[n].value = node->value;
			nodes[n].left = (nodes[n].right = -1);
		}
		n++;
	}
	QueueFree(q);
}

static int ObtainBytes_n(unsigned char *bytes, int nr_bytes, char** codes) {
	int i, s = 0;

	for(i=0; i<nr_bytes; i++)
		s += strlen(codes[bytes[i]]);
	
	return s/8 + ((s % 8 == 0) ? 0 : 1);
}

/* Write FileEntryHeaders and their codes to 
 * files; also obtain the compressed codes
 */
static void WriteFiles(HuffmanArchive *HA, int argc, FILE **f,
                       char **codes, FILE *farchive) {
	int i, file_len, j, l, k, incr = 0;
	unsigned char *bytes, byte = 0, bit;
	int bits_n = 0;
	
	for(i=0; i<argc-3; i++) {
		if (f[i] == NULL)
			continue;

		fseek(f[i], 0, SEEK_END);
		file_len = ftell(f[i]);
		rewind(f[i]);

		/* read all bytes from a file */
		bytes = calloc (file_len, sizeof(char));
		fread(bytes, sizeof(char), file_len, f[i]);
		fclose(f[i]);

		/* write FileEntryHeader */
		HA->fh[incr].bytes_n = ObtainBytes_n(bytes, file_len, codes);
		fwrite(&HA->fh[incr], sizeof(struct _FileEntryHeader), 1, farchive);

		/* form a byte from char codes; no padding */
		for(j=0; j<file_len; j++) {
			l = strlen(codes[bytes[j]]);
			for(k=0; k<l; k++) {
				if (bits_n == 8) {
					bits_n = 0;
					fwrite(&byte, sizeof(unsigned char), 1, farchive);
					byte = 0;
				}
				/* fill a byte and write it to file when full */
				bit = codes[bytes[j]][k] - '0';
				bit <<= ( 8 - ++bits_n );
				byte |= bit;
			}
		}
		if (bits_n > 0 && file_len)
			fwrite(&byte, sizeof(unsigned char), 1, farchive);
		byte = 0;
		bits_n = 0;
		incr++;
		free(bytes);
	}
}

/* Write the formed HuffmanArchive structure 
 */
static void WriteArchive(HuffmanArchive *HA, int argc, char **argv,
                          FILE **f, char **codes) {
	int i;
	FILE *farchive = fopen(argv[2], "wb");

	fwrite(&HA->ah, sizeof(struct _ArchiveHeader), 1, farchive);
	for(i=0; i<HA->ah.huffman_nodes_n; i++)
		fwrite(&HA->nodes[i], sizeof(struct _HuffmanNode), 1, farchive);
	WriteFiles(HA, argc, f, codes, farchive);
	fclose(farchive);
}

/* Find how many times characters from all
 * files repeat; also calculate checksum
 */
static void FormNodes(Node *n, int argc, FILE **f, HuffmanArchive *HA) {
	unsigned char *byte;
	int i, j, bytes, incr = 0;
	
	for(i=0; i<256; i++)
		n[i].value = i;

	for(i=0; i<argc-3; i++) {
		if (f[i] == NULL)
			continue;
		bytes = FileGetSize(f[i]);

		HA->fh[incr].file_size = bytes;
		HA->fh[incr].checksum = 0;
		
		byte = calloc (bytes, sizeof(unsigned char));
		fread(byte, sizeof(unsigned char), bytes, f[i]);

		/* read each byte of file */
		for(j=0; j<bytes; j++) {
			n[byte[j]].repeat++;
			HA->fh[incr].checksum += byte[j];
		}
		incr++;
		free(byte);
	}
}

static void AddNodesToHeap(Heap *h, Node *n) {
	int i;

	/* add the nodes for the HuffmanTree into heap */
	for(i=0; i<256; i++)
		if (n[i].repeat)
			HeapInsert(h, &n[i], CompareByRepeat);
}

static void HuffmanTreeCreate(Heap *h, Node *n, HuffmanArchive *HA) {
	Node *n1, *n2;
	int c = 256;
	
	/* treat this case separately; add another node
	so that a code be generated if only one file with 
	size 1 is inserted */
	if (HA->ah.huffman_nodes_n == 1) {
		n1 = HeapExtractMin(h, CompareByRepeat);
		n[c].repeat = 0;
		n[c].left = n1;
		HeapInsert(h, n+c, CompareByRepeat);
		HA->ah.huffman_nodes_n++;
	} else
		/* not a special with 1 byte then */
		while (HeapHasMoreThanOneElem(h)) {
			n1 = HeapExtractMin(h, CompareByRepeat);
			n2 = HeapExtractMin(h, CompareByRepeat);
			HA->ah.huffman_nodes_n++;
			
			/* insert a parent for n1, n2 into the heap */
			n[c].left = n1;
			n[c].right = n2;
			n[c].repeat = n1->repeat + n2->repeat;
			HeapInsert(h, n+c, CompareByRepeat);
			c++;
		}
}

void HuffmanCompress(int argc, char *argv[]) {
	char code[256] = "", *codes[256] = {0};
	Node n[511] = {{0,0,0,0}}, *root;
	HuffmanArchive HA;
	FILE **f;
	Heap *h;

	if (argc < 4) {
		PRINT_USAGE;
		exit(1);
	}
	f = calloc (argc-3, sizeof(FILE*));
	memset(&HA, 0, sizeof(HuffmanArchive));

	/* initialise HuffmanArchive and nodes */
	OpenFiles(f, &HA, argc, argv);
	FormNodes(n, argc, f, &HA);

	/* add the nodes to heap */
	h = HeapCreate(HEAP_SIZE_DF);
	AddNodesToHeap(h, n);

	HA.ah.huffman_root_node = 0;
	HA.ah.huffman_nodes_n = HeapGetSize(h);
	// and update the size while new nodes appear

	/* form the HuffmanTree */
	HuffmanTreeCreate(h, n, &HA);
	root = HeapExtractMin(h, CompareByRepeat);

	/* update the structures to be written 
	to file and write them */
	ObtainCodifications(root, code, codes);
	UpdateNodes(root, HA.nodes);
	WriteArchive(&HA, argc, argv, f, codes);
	// also closes the files opened

	HeapFree(h);
	FreeCodes(codes);
	free(HA.fh);
	free(f);
}

static unsigned char DecodeByte(HuffmanNode *nodes, int root, 
                                unsigned char *bytes, int *bit) {
	int element, shift;
	unsigned char mask;

	while (nodes[root].left != -1 || nodes[root].right != -1) {
		/* find the corresponding bit
		 * to tell me left/right -> 0/1 
		 */
		shift = 7 - (*bit % 8);
		mask = 1 << shift;
		element = ( mask & bytes[*bit/8] ) >> shift;

		root = (element == 0)? nodes[root].left : nodes[root].right;
		(*bit)++;
	}
	return nodes[root].value;
}

static void DecodeAndWriteFiles(HuffmanArchive HA, char *folder_out, FILE *ar) {
	unsigned char *read_bytes, *write_bytes;
	int i, j, root, bit, offset, nodes_n, size;
	unsigned int checksum;
	char *file_out = NULL;
	FILE *f;

	root = HA.ah.huffman_root_node;
	nodes_n = HA.ah.huffman_nodes_n;

	/* move to part where file details are */
	offset = sizeof(ArchiveHeader) + nodes_n * sizeof(HuffmanNode);
	fseek(ar, offset, SEEK_SET);

	size = ((folder_out)?strlen(folder_out):0) + MAX_FILENAME_SIZE + 2;

	/* here we'll write all files */
	for(i=0; i<HA.ah.file_entries_n; i++) {
		bit = 0;
		checksum = 0;
		fseek(ar, sizeof(FileEntryHeader), SEEK_CUR);

		/* compose the name of the file to be open */
		file_out = calloc (size, sizeof(char));
		if (folder_out)
			sprintf(file_out, "%s/%s", folder_out, HA.fh[i].file_name);
		else
			strcpy(file_out, HA.fh[i].file_name);
		f = fopen(file_out, "w");

		if (f == NULL) {
			fprintf(stderr, "Error: Can't open %s\n", folder_out);
			fseek(ar, HA.fh[i].bytes_n, SEEK_CUR);
			free(file_out);
			continue;
		}
		read_bytes = calloc (HA.fh[i].bytes_n, sizeof(unsigned char));
		write_bytes = calloc (HA.fh[i].file_size, sizeof(unsigned char));
		fread(read_bytes, sizeof(unsigned char), HA.fh[i].bytes_n, ar);

		/* decompress all bytes */
		for(j=0; j<HA.fh[i].file_size; j++) {
			write_bytes[j] = DecodeByte(HA.nodes, root, read_bytes, &bit);
			checksum += write_bytes[j];
		}
		free(read_bytes);

		/* write the file, if checksum is correct */
		if (checksum != HA.fh[i].checksum)
			fprintf(stderr, "Error: %s is CORRUPT\n", HA.fh[i].file_name);
		else
			fwrite(write_bytes, sizeof(unsigned char), HA.fh[i].file_size, f);
		fclose(f);
		free(write_bytes);
		free(file_out);
	}
}

static void FillArchive(HuffmanArchive *HA, FILE *ar, char *archive) {
	int i, nr;

	nr = fread(&HA->ah, sizeof(ArchiveHeader), 1, ar);
	if (nr != 1) {
		fprintf(stderr, "Error: %s is CORRUPT\n", archive);
		exit(1);
	}
	HA->fh = calloc (HA->ah.file_entries_n, sizeof(FileEntryHeader));

	for(i=0; i<HA->ah.huffman_nodes_n; i++)
		fread(&HA->nodes[i], sizeof(HuffmanNode), 1, ar);
	
	/* for each file, read details */
	for(i=0; i<HA->ah.file_entries_n; i++) {
		fread(&HA->fh[i], sizeof(FileEntryHeader), 1, ar);
		/* skip compressed code */
		fseek(ar, HA->fh[i].bytes_n, SEEK_CUR);
	}
}

void HuffmanExtract(int argc, char *argv[]) {
	FILE *archive;
	HuffmanArchive HA;

	if (argc < 3) {
		PRINT_USAGE;
		exit(1);
	}
	archive = fopen(argv[2], "rb");
	FillArchive(&HA, archive, argv[2]);
	DecodeAndWriteFiles(HA, argv[3], archive);

	fclose(archive);
	free(HA.fh);
}

void HuffmanList(int argc, char *argv[]) {
	HuffmanArchive HA;
	FILE *arf; //archive file
	int i;

	if (argc != 3) {
		PRINT_USAGE;
		exit(1);
	}
	arf = fopen(argv[2], "rb");
	if (arf == NULL) {
		fprintf(stderr, "Error: Can't open %s\n", argv[2]); 
		exit(1);
	}
	/* read ArchiveHeader */
	fread(&HA.ah, sizeof(ArchiveHeader), 1, arf);

	/* skip HuffmanNodes */
	fseek(arf, HA.ah.huffman_nodes_n * sizeof(HuffmanNode), SEEK_CUR);
	HA.fh = calloc (HA.ah.file_entries_n, sizeof(FileEntryHeader));

	/* read FileEntryHeaders and skip compressed codes */
	for(i=0; i<HA.ah.file_entries_n; i++) {
		fread(&HA.fh[i], sizeof(FileEntryHeader), 1, arf);
		fseek(arf, HA.fh[i].bytes_n, SEEK_CUR);
	}

	/* sort the output */
	qsort(HA.fh, HA.ah.file_entries_n, sizeof(FileEntryHeader), MyStrcmp);
	for(i=0; i<HA.ah.file_entries_n; i++)
		printf("%-30s %d\n", HA.fh[i].file_name, HA.fh[i].file_size);

	fclose(arf);
	free(HA.fh);
}
