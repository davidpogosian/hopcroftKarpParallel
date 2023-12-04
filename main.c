#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#include "generateGraph.h"
#include "printMatrix.h"
#include "hopcroftKarpParallel.h"

#define RANDOM_GRAPH 1

#define PRINT_INTPUT_OUTPUT 0

#define MIN_ROWS 400
#define MIN_COLS 400
#define MAX_ROWS 600
#define MAX_COLS 600

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	double start_time, end_time;
	int world_size, world_rank;
	int rows, cols;
	int* graph;
	int* max_matching;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	if (world_rank == 0) {
		if (RANDOM_GRAPH) {
			srand(time(NULL));
			rows = MIN_ROWS + rand() % (MAX_ROWS - MIN_ROWS + 1);
			cols = MIN_COLS + rand() % (MAX_COLS - MIN_COLS + 1);
			graph = generateGraph(0.3, rows, cols);
		} else {
			/*
			test case 1: where bfs doesn't work 
			0 0 0 1 0 0 
			0 1 0 1 1 0 
			0 1 0 0 0 0 
			1 1 1 0 1 0 
			0 0 0 0 0 0 
			*/
			rows = 5;
			cols = 6;
			graph = (int*) malloc(sizeof(int) * (rows * cols));
			graph[0] = 0;
			graph[1] = 0;
			graph[2] = 0;
			graph[3] = 1;
			graph[4] = 0;
			graph[5] = 0;
			/**/
			graph[6] = 0;
			graph[7] = 1;
			graph[8] = 0;
			graph[9] = 1;
			graph[10] = 1;
			graph[11] = 0;
			/**/
			graph[12] = 0;
			graph[13] = 1;
			graph[14] = 0;
			graph[15] = 0;
			graph[16] = 0;
			graph[17] = 0;
			/**/
			graph[18] = 1;
			graph[19] = 1;
			graph[20] = 1;
			graph[21] = 0;
			graph[22] = 1;
			graph[23] = 0;
			/**/
			graph[24] = 0;
			graph[25] = 0;
			graph[26] = 0;
			graph[27] = 0;
			graph[28] = 0;
			graph[29] = 0;
			/**/
		}
		if (PRINT_INTPUT_OUTPUT) {
			printf("input matrix: \n");
			printMatrix(graph, rows, cols);
		}
	}

	/* start stopwatch */
	if (world_rank == 0) {
		start_time = MPI_Wtime();
	}
	MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (world_rank != 0) {
		graph = (int*) malloc(sizeof(int) * (rows * cols));
	}
	MPI_Bcast(graph, rows * cols, MPI_INT, 0, MPI_COMM_WORLD);

	// maximal alg

	// maximum alg
	max_matching = hopcroftKarpParallel(graph, rows, cols, world_rank, world_size);	
	
	/* stop stopwatch and display result */
	if (world_rank == 0) {
		end_time = MPI_Wtime();
		if (PRINT_INTPUT_OUTPUT) {
			printf("output matrix: \n");
			printMatrix(max_matching, rows, cols);
		}
		printf("elapsed time: %f \n", end_time - start_time);
	}
	if (RANDOM_GRAPH) {
		free(graph);
	}
	free(max_matching);
	MPI_Finalize();
	return 0;
}
