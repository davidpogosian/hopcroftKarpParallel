

#include "IntList.h"
#include "Queue.h"

void Queue_init(Queue* queue) {
    IntList_init(&(queue->int_list));
}

int Queue_dequeue(Queue* queue) {
    return IntList_removeByPosition(&(queue->int_list), 0);
}

void Queue_enqueue(Queue* queue, int new_element) {
    IntList_appendToEnd(&(queue->int_list), new_element);
}

int Queue_isEmpty(Queue* queue) {
    return IntList_isEmpty(&(queue->int_list));
}

void Queue_dealloc(Queue* queue) {
    IntList_dealloc(&(queue->int_list));
}

void Queue_print(Queue* queue, int tag) {
    IntList_print(&(queue->int_list), tag);
}