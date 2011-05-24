#include "queue.h"
#include <stdlib.h>

Queue QueueCreate() {
	Queue q = calloc (1, sizeof(struct queue));
	return q;
}

void Enqueue(Queue q, void *data) {
	if (q == NULL)
		return;
	QNode new = calloc (1, sizeof(struct node));
	new->data = data;

	if (q->size == 0) {
		q->front = (q->back = new);
		q->size++;
		return;
	}
	q->back->next = new;
	q->back = new;
	q->size++;
}

void *Dequeue(Queue q) {
	if (q == NULL || q->size == 0)
		return NULL;
	QNode del = q->front;
	void *d;

	q->size--;
	q->front = del->next;
	d = del->data;

	free(del);
	return d;
}

int QueueSize(Queue q) {
	return q->size;
}

int QueueEmpty(Queue q) {
	return q->size == 0;
}

void QueueFree(Queue q) {
	if (q == NULL)
		return;
	free(q);
}
