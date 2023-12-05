#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#include "generateGraph.h"
#include "printMatrix.h"
#include "hopcroftKarpParallel.h"
#include "hopcroftKarpSequential.h"
#include "tools.h"

#define PRINT_INTPUT_OUTPUT 0
#define RUN_SEQUENTIAL 1

#define MIN_ROWS 100
#define MIN_COLS 100
#define MAX_ROWS 200
#define MAX_COLS 200

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	double start_time_parallel, end_time_parallel, start_time_sequential, end_time_sequential;
	int world_size, world_rank;
	int rows, cols;
	int* graph;
	int* max_matching_parallel;
	int* max_matching_sequential;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	for (int i = 0; i < 1000; ++i) {
		if (world_rank == 0) {
			srand(i);
			rows = MIN_ROWS + rand() % (MAX_ROWS - MIN_ROWS + 1);
			cols = MIN_COLS + rand() % (MAX_COLS - MIN_COLS + 1);
			graph = generateGraph(0.3, rows, cols);		
			if (PRINT_INTPUT_OUTPUT) {
				printf("input matrix: \n");
				printMatrix(graph, rows, cols);
			}
		}

		/* start stopwatch */
		if (world_rank == 0) {
			start_time_parallel = MPI_Wtime();
		}
		MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (world_rank != 0) {
			graph = (int*) malloc(sizeof(int) * (rows * cols));
		}
		MPI_Bcast(graph, rows * cols, MPI_INT, 0, MPI_COMM_WORLD);

		max_matching_parallel = hopcroftKarpParallel(graph, rows, cols, world_rank, world_size);	
		
		/* stop stopwatch and display result */
		if (world_rank == 0) {
			end_time_parallel = MPI_Wtime();
			if (PRINT_INTPUT_OUTPUT) {
				printf("parallel output matrix: \n");
				printMatrix(max_matching_parallel, rows, cols);
			}

			/* run sequential counterpart */
			if (RUN_SEQUENTIAL) {
				start_time_sequential = MPI_Wtime();
				max_matching_sequential = hopcroftKarpSequential(graph, rows, cols);
				end_time_sequential = MPI_Wtime();
			}
			
			if (RUN_SEQUENTIAL && PRINT_INTPUT_OUTPUT) {
				printf("sequential output matrix: \n");
				printMatrix(max_matching_sequential, rows, cols);
			}

			if (RUN_SEQUENTIAL) {
				printf("(%dx%d)s vs p: %f vs %f \n", rows, cols, end_time_sequential - start_time_sequential, end_time_parallel - start_time_parallel);

				if (!verifyMatchingProperty(max_matching_sequential, rows, cols)) {
					printf("ERROR :: matching property violated (sequential) \n");
					printf("graph: \n");
					printMatrix(graph, rows, cols);
					printf("matching: \n");
					printMatrix(max_matching_sequential, rows, cols);
					break;
				}
			}
			if (!verifyMatchingProperty(max_matching_parallel, rows, cols)) {
				printf("ERROR :: matching property violated (parallel) \n");
				printf("graph: \n");
				printMatrix(graph, rows, cols);
				printf("matching: \n");
				printMatrix(max_matching_parallel, rows, cols);
				break;
			}
			if (RUN_SEQUENTIAL) {
				if (getCardinality(max_matching_sequential, rows, cols) != getCardinality(max_matching_parallel, rows, cols)) {
					printf("ERROR :: cardinalities don't match \n");
					printMatrix(graph, rows, cols);
					break;
				}
			}
			free(max_matching_sequential);
		}
		free(graph);
		free(max_matching_parallel);
		
	}

	MPI_Finalize();
	return 0;
}
