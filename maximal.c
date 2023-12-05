#include <stdlib.h>



int* maximal(int* graph, int rows, int cols) {
    int* maximal_matching = (int*) malloc(sizeof(int) * (rows * cols));
    int* vertex_right_free = (int*) malloc(sizeof(int) * cols);
    for (int j = 0; j < cols; ++j) {
        vertex_right_free[j] = 1;
    }
    for (int i = 0; i < rows; ++i) {
        int vertex_left_free = 1;
        for (int j = 0; j < cols; ++j) {
            maximal_matching[i * cols + j] = 0;
            if (graph[i * cols + j]) {
                if (vertex_right_free[j] && vertex_left_free) {
                    vertex_right_free[j] = 0;
                    vertex_left_free = 0;
                    maximal_matching[i * cols + j] = 1;
                }
            }
        }
    }
    free(vertex_right_free);
    return maximal_matching;
}