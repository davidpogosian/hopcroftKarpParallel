#ifndef INT_LIST_H
#define INT_LIST_H

typedef struct {
    int* arr;
    int tail;
    int length;
} IntList;

void IntList_init(IntList* int_list);
void IntList_appendToEnd(IntList* int_list, int new_element);
int IntList_removeByPosition(IntList* int_list, int position);
void IntList_dealloc(IntList* int_list);
void IntList_print(IntList* int_list, int tag);
int IntList_isEmpty(IntList* int_list);
int IntList_getLength(IntList* int_list);
void IntList_shrinkToFit(IntList* int_list);

#endif