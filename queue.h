#ifndef queue_h
#define queue_h

typedef struct node {
	void *data;
	struct node *next;
} *QNode;

typedef struct queue {
	int size;
	struct node *front;
	struct node *back;
} *Queue;

Queue QueueCreate();
void Enqueue(Queue q, void *data);
void *Dequeue(Queue q);
int QueueSize(Queue q);
int QueueEmpty(Queue q);
void QueueFree(Queue q);

#endif
