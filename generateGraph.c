#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "generateGraph.h"

int* generateGraph(double p, int rows, int cols) {
	srand(time(NULL));
	int* graph = (int*) malloc(sizeof(int) * rows * cols);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if ((double) rand() / RAND_MAX <= p) {
				graph[i*cols + j] = 1;
			} else {
				graph[i*cols + j] = 0;
			}
		}
	}
	return graph;
}
