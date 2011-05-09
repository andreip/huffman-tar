#ifndef heap_h
#define heap_h

#define PARENT(i)		((i-1)/2)
#define LEFT(i)			(2*i+1)
#define RIGHT(i)		(2*i+2)
#define COMP(a,b)		fcomp(h->values[a], h->values[b])
#define HEAP_SIZE_DF	200
//#define SWAP(a,b)	{ h->values[a] 

/* pointer to comparison/swap function */
typedef int	(*PFC)(void *,void *);

typedef struct heap {
	void **values;
	int dimV;
	int capV;
} Heap;

Heap *HeapCreate(int size);
void HeapFree(Heap *h);
/* min-heapify */
void HeapInsert(Heap *h, void *v, PFC fcomp);
void *HeapReturnMin(Heap *h);
void *HeapExtractMin(Heap *h, PFC fcomp);
int HeapHasMoreThanOneElem(Heap *h);
int HeapGetSize(Heap *h);

#endif
