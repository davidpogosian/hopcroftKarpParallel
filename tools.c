#include <stdlib.h>
#include <stdio.h>



int verifyMatchingProperty(int* matching, int rows, int cols) {
    int* columns_with_ones = (int*) malloc(sizeof(int) * cols);
    for (int j = 0; j < cols; ++j) {
        columns_with_ones[j] = 0;
    }
    for (int i = 0; i < rows; ++i) {
        int row_has_one = 0;
        for (int j = 0; j < cols; ++j) {
            if (matching[i * cols + j]) {
                if (columns_with_ones[j]) {
                    printf("%d %d \n", i, j);
                    return 0;
                }
                if (row_has_one) {
                    printf("%d %d \n", i, j);
                    return 0;
                }
                row_has_one = 1;
                columns_with_ones[j] = 1;
            }
        }
    }
    free(columns_with_ones);
    return 1;
}

int getCardinality(int* matching, int rows, int cols) {
    int edges = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matching[i * cols + j]) {
                edges += 1;
            }
        }
    }
    return edges;
}