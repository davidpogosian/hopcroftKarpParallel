#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#include "generateGraph.h"
#include "printMatrix.h"
#include "hopcroftKarpParallel.h"
#include "hopcroftKarpSequential.h"
#include "tools.h"
#include "maximal.h"

#define PRINT_INTPUT_OUTPUT 0
#define RUN_SEQUENTIAL 1
#define NUMBER_OF_TESTS 5
int USE_MAXIMAL = 1;


#define MIN_ROWS 500
#define MIN_COLS 500
#define MAX_ROWS 1000
#define MAX_COLS 1000

FILE* fp;

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	double start_time_parallel, end_time_parallel, start_time_sequential, end_time_sequential;
	int world_size, world_rank;
	int rows, cols;
	int* graph;
	int* max_matching_parallel;
	int* max_matching_sequential;
	int* maximal_matching = NULL;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (world_rank == 0) {
		fp = fopen("graph_data.txt", "w");
	}

	
	for (int p = 0.1; p < 1.0; p += 0.1) {
		double parallel_time_sum = 0;
		double sequential_time_sum = 0;
		/* run each test five times and average the result */
		for (int rep = 0; rep < NUMBER_OF_TESTS; ++rep) {
			if (world_rank == 0) {
				srand(rep + time(NULL));
				rows = MIN_ROWS + rand() % (MAX_ROWS - MIN_ROWS + 1);
				cols = MIN_COLS + rand() % (MAX_COLS - MIN_COLS + 1);
				graph = generateGraph((double) (rep/100), rows, cols);	
				if (USE_MAXIMAL) {
					maximal_matching = maximal(graph, rows, cols);
				}	
				if (PRINT_INTPUT_OUTPUT) {
					printf("input matrix: \n");
					printMatrix(graph, rows, cols);
					if (USE_MAXIMAL) {
						printf("maximal matching: ");
						printMatrix(maximal_matching, rows, cols);
					}
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
				if (USE_MAXIMAL) {
					maximal_matching = (int*) malloc(sizeof(int) * (rows * cols));
				}
			}
			MPI_Bcast(graph, rows * cols, MPI_INT, 0, MPI_COMM_WORLD);
			if (USE_MAXIMAL) {
				MPI_Bcast(maximal_matching, rows * cols, MPI_INT, 0, MPI_COMM_WORLD);
			}
			
			max_matching_parallel = hopcroftKarpParallel(graph, rows, cols, world_rank, world_size, maximal_matching);	
			
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
					max_matching_sequential = hopcroftKarpSequential(graph, rows, cols, maximal_matching);
					end_time_sequential = MPI_Wtime();
				}
				
				if (RUN_SEQUENTIAL && PRINT_INTPUT_OUTPUT) {
					printf("sequential output matrix: \n");
					printMatrix(max_matching_sequential, rows, cols);
				}

				if (RUN_SEQUENTIAL) {
					printf("(%dx%d) s vs p: %f vs %f \n", rows, cols, end_time_sequential - start_time_sequential, end_time_parallel - start_time_parallel);

					parallel_time_sum += end_time_parallel - start_time_parallel;
					sequential_time_sum += end_time_sequential - start_time_sequential;

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
			if (USE_MAXIMAL) {
				free(maximal_matching);
			}
			free(max_matching_parallel);
		}

		double parallel_time = parallel_time_sum / NUMBER_OF_TESTS;
		double sequential_time = sequential_time_sum / NUMBER_OF_TESTS;
		/* write into data file */
		fprintf(fp, "%f %f %f \n",(double) p, sequential_time, parallel_time);
	}
	fclose(fp);
	MPI_Finalize();
	return 0;
}

