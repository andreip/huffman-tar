/*
 * Nume  : Petre Andrei
 * Grupa : 315 CA
 */
#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ResolveTask(int argc, char *argv[]) {
	if (argc < 3) {
		PRINT_USAGE;
		exit(1);
	}
	
	if ( !strcmp(argv[1], "compress") )
		HuffmanCompress(argc, argv);
	else if ( !strcmp(argv[1], "list") )
		HuffmanList(argc, argv);
	else if ( !strcmp(argv[1], "extract") )
		HuffmanExtract(argc, argv);
	else {
		PRINT_USAGE;
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	ResolveTask(argc, argv);
	return 0;
}
