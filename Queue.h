#ifndef QUEUE_H
#define QUEUE_H

#include "IntList.h"

typedef struct {
    IntList int_list;
} Queue;

void Queue_init(Queue* queue);
int Queue_dequeue(Queue* queue);
void Queue_enqueue(Queue* queue, int new_element);
int Queue_isEmpty(Queue* queue);
void Queue_dealloc(Queue* queue);
void Queue_print(Queue* queue, int tag);

#endif
