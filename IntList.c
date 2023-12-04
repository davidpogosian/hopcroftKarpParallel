#include <stdlib.h>
#include <stdio.h>

#include "IntList.h"

void IntList_init(IntList* int_list) {
    int_list->arr = (int*) malloc(sizeof(int) * 4);
    int_list->length = 4;
    int_list->tail = 0;
}

void IntList_appendToEnd(IntList* int_list, int new_element) {
    if (int_list->tail == int_list->length) {
        int* new_arr = (int*) malloc(sizeof(int) * int_list->length * 2);
        for (int i = 0; i < int_list->length; ++i) {
            new_arr[i] = int_list->arr[i];
        }
        free(int_list->arr);
        int_list->arr = new_arr;
        int_list->length *= 2;
    }
    int_list->arr[int_list->tail] = new_element;
    int_list->tail += 1;
}

int IntList_removeByPosition(IntList* int_list, int position) {
    if (position >= int_list->tail) {
        printf("!!! int_list removing past the tail \n");
        return -1;
    }
    int removed_value = int_list->arr[position];
    int_list->arr[position] = 0;
    for (int i = position; i < int_list->tail - 1; ++i) {
        int_list->arr[i] = int_list->arr[i + 1];
    }
    int_list->arr[int_list->tail] = 0;
    int_list->tail -= 1;
    return removed_value;
}

void IntList_dealloc(IntList* int_list) {
    free(int_list->arr);
}

void IntList_print(IntList* int_list, int tag) {
    printf("~ \n");
    for (int i = 0; i < int_list->tail; ++i) {
        printf("%d: %d \n", tag, int_list->arr[i]);
    }
    printf("~ \n");
}

int IntList_isEmpty(IntList* int_list) {
    return int_list->tail == 0;
}

int IntList_getLength(IntList* int_list) {
    return int_list->tail;
}

void IntList_shrinkToFit(IntList* int_list) {
    if (int_list->tail != int_list->length) {
        int* new_arr = (int*) malloc(sizeof(int) * int_list->tail);
        for (int i = 0; i < int_list->tail; ++i) {
            new_arr[i] = int_list->arr[i];
        }
        free(int_list->arr);
        int_list->arr = new_arr;
        int_list->length = int_list->tail;
    }
}