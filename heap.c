#include "heap.h"
#include <stdlib.h>

Heap *HeapCreate(int size) {
	Heap *h = calloc (1, sizeof(struct heap));

	h->values = calloc (size, sizeof(void*));
	h->capV = size;

	return h;
}

void HeapFree(Heap *h) {
	free(h->values);
	free(h);
}

static void HeapHeapifyDown(Heap *h, int pos, PFC fcomp) {
	int l = LEFT(pos);
	int r = RIGHT(pos);
	int min;
	void *aux;

	if (pos >= h->dimV || h->dimV < 2)
		return;

	if ( (l < h->dimV) && (COMP(l, pos) < 0) )
		min = l;
	else
		min = pos;
	if ( (r < h->dimV) && (COMP(r, min) < 0) )
		min = r;
	if (min != pos) {
		aux = h->values[min];
		h->values[min] = h->values[pos];
		h->values[pos] = aux;
		HeapHeapifyDown(h, min, fcomp);
	}
}

static void HeapHeapifyUp(Heap *h, int pos, PFC fcomp) {
	if (pos >= h->dimV || pos < 1)
		return;
	while (pos > 0 && COMP(pos, PARENT(pos)) < 0) {
		void *aux = h->values[pos];
		h->values[pos] = h->values[PARENT(pos)];
		h->values[PARENT(pos)] = aux;
		pos = PARENT(pos);
	}
}

void HeapInsert(Heap *h, void *v, PFC fcomp) {
	if (h == NULL || v == NULL)
		return;
	if (h->dimV == h->capV) {
		h->capV *= 2;
		h->values = realloc (h->values, h->capV * sizeof(struct heap));
	}

	h->values[h->dimV++] = v;
	HeapHeapifyUp(h, h->dimV-1, fcomp);
}

void *HeapReturnMin(Heap *h) {
	if (h == NULL)
		return NULL;
	return h->values[0];
}

void *HeapExtractMin(Heap *h, PFC fcomp) {
	if (h == NULL || h->dimV < 1)
		return NULL;
	void *min = h->values[0];

	h->dimV--;
	if (h->dimV > 0) {
		h->values[0] = h->values[h->dimV];
		HeapHeapifyDown(h, 0, fcomp);
	}
	return min;
}

int HeapHasMoreThanOneElem(Heap *h) {
	return h->dimV > 1;
}

int HeapGetSize(Heap *h) {
	return h->dimV;
}
