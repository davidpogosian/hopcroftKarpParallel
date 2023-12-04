#include <stdio.h>

#include "printMatrix.h"

void printMatrix(int* matrix, int rows, int cols) {
	printf("~ \n");
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%d ", matrix[i*cols + j]);
		}
		printf("\n");
	}
	printf("~ \n");
}
