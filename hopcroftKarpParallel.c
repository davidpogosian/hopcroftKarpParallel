#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "hopcroftKarpParallel.h"
#include "Queue.h"
#include "IntList.h"

#define DEBUG_LEFT_VERTICES_PARTITIONING 0
#define DEBUG_BFS 0
#define DEBUG_AUGMENTING_PATHS_LOCAL 0
#define DEBUG_NUMBER_OF_PATHS_DISTRIBUTION 0
#define DEBUG_GATHERV 0
#define DEBUG_AUGMENTING_PATHS_GLOBAL 0
#define DEBUG_VERTEX_DISJOINT_ALG 0

int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}

int* hopcroftKarpParallel(int* graph, int rows, int cols, int world_rank, int world_size) {
	/* initialize matching */
	int* current_matching = (int*) malloc(sizeof(int) * (rows * cols)); /* freed in main.c */
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			current_matching[i*cols + j] = 0;
		}
	}
	
	while (1) {
		/* initialize vertices_right_matches using current_matching */
		int* vertices_right_matches = (int*) malloc(sizeof(int) * cols);
		for (int j = 0; j < cols; ++j) {
			int match = -1;
			for (int i = 0; i < rows; ++i) {
				if (current_matching[i*cols + j] == 1) {
					match = i;
					break;
				}
			}
			vertices_right_matches[j] = match;
		}

		/* find all free vertices in the left partition >> vertices_left_free_buffer */
		int* vertices_left_free_buffer = (int*) malloc(sizeof(int) * rows);
		int index = 0;
		for (int i = 0; i < rows; ++i) {
			int empty_row = 1;
			for (int j = 0; j < cols; ++j) {
				if (current_matching[i*cols + j] == 1) {
					empty_row = 0;
					break;
				}
			}
			if (empty_row == 1) {
				vertices_left_free_buffer[index] = i;
				index += 1;
			}
		}

		/* how many vertices per process? which vertices are mine? */
		int total_number_vertices_left_free = index;
		int quotient_vertices_left_free_per_process = total_number_vertices_left_free / world_size;
		int remainder_vertices_left_free_per_process = total_number_vertices_left_free % world_size;
		int my_number_vertex_left_free = quotient_vertices_left_free_per_process + (world_rank < remainder_vertices_left_free_per_process);
		int my_first_vertex_left_free = world_rank * quotient_vertices_left_free_per_process + min(world_rank, remainder_vertices_left_free_per_process);
		int* my_vertices_left_free;
		my_vertices_left_free = (int*) malloc(sizeof(int) * my_number_vertex_left_free);
		index = 0;
		for (int i = my_first_vertex_left_free; i < my_first_vertex_left_free + my_number_vertex_left_free; ++i) {
			my_vertices_left_free[index] = vertices_left_free_buffer[i];
			index += 1;
		}
		
		if (DEBUG_LEFT_VERTICES_PARTITIONING) {
			printf("rank: %d total_number_vertices_left_free: %d \n", world_rank, total_number_vertices_left_free);
			printf("rank: %d my_number_vertex_left_free: %d \n", world_rank, my_number_vertex_left_free);
			for (int i = 0; i < my_number_vertex_left_free; ++i) {
				printf("rank: %d -> vertex left free: %d \n", world_rank, my_vertices_left_free[i]);
			}
		}

		/* SS-BFS find shortest a.p. for each vertex in my_vertices_left_free */
		int local_min_length = __INT_MAX__;
		IntList augmenting_paths;
		IntList augmenting_path_lengths;
		IntList_init(&augmenting_paths);
		IntList_init(&augmenting_path_lengths);
		for (int i = 0; i < my_number_vertex_left_free; i++) {
			int matching_vertex = -1;
			int* explored_right = (int*) malloc(sizeof(int) * cols);
			int* parent_left = (int*) malloc(sizeof(int) * rows); /* [left_vertex] => right_vertex */
			int* parent_right = (int*) malloc(sizeof(int) * cols); /* [right_vertex] => left_vertex */
			for (int _ = 0; _ < rows; ++_) {
				parent_left[_] = 0;
			}
			for (int _ = 0; _ < cols; ++_) {
				explored_right[_] = 0;
				parent_right[_] = 0;
			}
			Queue my_queue;
			Queue_init(&my_queue);
			Queue_enqueue(&my_queue, my_vertices_left_free[i]);
			if (DEBUG_BFS) {
				printf("%d, strating search based out of (left) %d \n", world_rank, my_vertices_left_free[i]);
			}
			while (!Queue_isEmpty(&my_queue)) {
				int current_vertex = Queue_dequeue(&my_queue);
				if (DEBUG_BFS) {
					printf("%d, visiting (left) %d \n", world_rank, current_vertex);
				}
				for (int j = 0; j < cols; ++j) {
					if (graph[current_vertex*cols + j]) {
						if (!explored_right[j]) {
							parent_right[j] = current_vertex;
							if (vertices_right_matches[j] == -1) {
								/* right vertex isn't matched */
								matching_vertex = j;
								break;
							} else {
								/* right vertex is matched */
								parent_left[vertices_right_matches[j]] = j;
								explored_right[j] = 1;
								Queue_enqueue(&my_queue, vertices_right_matches[j]);
								if (DEBUG_BFS) {
									printf("%d, jump thru (right) %d \n", world_rank, j);
								}			
							}
						}
					}
				}
				if (matching_vertex != -1) {
					/* we found an aug path */
					break;
				}
			}
			if (matching_vertex != -1) {
				/* we found an aug path */
				if (DEBUG_BFS) {
					printf("%d, path found (to %d) \n", world_rank, matching_vertex);
				}
				int length = 1;
				int left_vertex = parent_right[matching_vertex];
				int right_vertex = matching_vertex;
				IntList_appendToEnd(&augmenting_paths, left_vertex);
				IntList_appendToEnd(&augmenting_paths, right_vertex);
				vertices_right_matches[right_vertex] = left_vertex;
				while (left_vertex != my_vertices_left_free[i]) {
					right_vertex = parent_left[left_vertex];
					IntList_appendToEnd(&augmenting_paths, left_vertex);
					IntList_appendToEnd(&augmenting_paths, right_vertex);
					left_vertex = parent_right[right_vertex];
					IntList_appendToEnd(&augmenting_paths, left_vertex);
					IntList_appendToEnd(&augmenting_paths, right_vertex);
					length += 2;
					vertices_right_matches[right_vertex] = left_vertex;
				}
				IntList_appendToEnd(&augmenting_path_lengths, length);
				if (length < local_min_length) {
					local_min_length = length;
				}
			} else {
				if (DEBUG_BFS) {
					printf("%d, no path \n", world_rank);
				}
			}
			free(explored_right);
			free(parent_left);
			free(parent_right);
			Queue_dealloc(&my_queue);
			if (DEBUG_BFS) {
				printf("%d, deallocs successful \n", world_rank);
			}
		}

		if (DEBUG_AUGMENTING_PATHS_LOCAL) {
			printf("%d: augmenting paths: \n", world_rank);
			IntList_print(&augmenting_paths, world_rank);
			printf("%d: a.p. lengths: \n", world_rank);
			IntList_print(&augmenting_path_lengths, world_rank);
		}

		/* find out what the minimum path length for this iteration is */
		int global_min_length = -1;
		MPI_Allreduce(&local_min_length, &global_min_length, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

		/* how many paths of global_min_length do i have? */
		int number_global_min_length_paths = 0;
		index = 0;
		IntList outgoing_paths;
		IntList_init(&outgoing_paths);
		for (int i = 0; i < IntList_getLength(&augmenting_path_lengths); ++i) {
			if (augmenting_path_lengths.arr[i] == global_min_length) {
				number_global_min_length_paths += 1;
				for (int j = index; j < index + global_min_length * 2; ++j) { // added "index + "
					IntList_appendToEnd(&outgoing_paths, augmenting_paths.arr[j]);
				}
			}
			index += augmenting_path_lengths.arr[i] * 2;
		}

		/* share number_global_min_length_paths with other procs */ // ALL TO ALL INSTEAD
		int* number_paths_per_process = (int*) malloc(sizeof(int) * world_size);
		number_paths_per_process[world_rank] = number_global_min_length_paths;
		for (int i = 0; i < world_size; ++i) {
			if (i == world_rank) {
				MPI_Bcast(&number_global_min_length_paths, 1, MPI_INT, world_rank, MPI_COMM_WORLD);
			} else {
				MPI_Bcast(&number_paths_per_process[i], 1, MPI_INT, i, MPI_COMM_WORLD);
			}
		}

		if (DEBUG_NUMBER_OF_PATHS_DISTRIBUTION) {
			if (world_rank == 0) {
				for (int i = 0; i < world_size; ++i) {
					printf("proc: %d -> paths: %d \n", i, number_paths_per_process[i]);
				}
			}
		}

		/* prepare to accept incoming paths */
		int number_of_edges_in_all_paths = 0;
		for (int i = 0; i < world_size; ++i) {
			number_of_edges_in_all_paths += number_paths_per_process[i] * global_min_length * 2;
		}

		/* exit condition */
		if (number_of_edges_in_all_paths == 0) {
			if (world_rank == 0) {
				// printf("COMPLETE \n");
			}
			break;
		}

		int* incoming_paths = (int*) malloc(sizeof(int) * number_of_edges_in_all_paths);
		for (int i = 0; i < world_size; ++i) {
			number_paths_per_process[i] *= 2 * global_min_length;
		}
		int* displacements = (int*) malloc(sizeof(int) * world_size);
		index = 0;
		displacements[0] = index;
		for (int i = 1; i < world_size; ++i) {
			index += number_paths_per_process[i - 1]; // rename n_p_p_p
			displacements[i] = index;
		}
		IntList_shrinkToFit(&outgoing_paths);

		/* gather all paths */
		MPI_Gatherv(outgoing_paths.arr, global_min_length * number_global_min_length_paths * 2, MPI_INT, incoming_paths, number_paths_per_process, displacements , MPI_INT, 0, MPI_COMM_WORLD);
	//									global_min_legnth * 		   					   * 2 added this to make it work
		if (DEBUG_GATHERV) {
			if (world_rank == 0) {
				printf("displacements: \n");
				for (int i = 0; i < world_size; ++i) {
					printf("%d \n", displacements[i]);
				}
				printf("incoming_paths: \n");
				for (int i = 0; i < number_of_edges_in_all_paths; ++i) {
					printf("%d \n", incoming_paths[i]);
				}
			} else {
				IntList_print(&outgoing_paths, world_rank);
			}
		}

		/* bcast all paths 0 -> everyone */
		MPI_Bcast(incoming_paths, number_of_edges_in_all_paths, MPI_INT, 0, MPI_COMM_WORLD);

		if (DEBUG_AUGMENTING_PATHS_GLOBAL) {
			if (world_rank == 1) {
				for (int i = 0; i < number_of_edges_in_all_paths; ++i) {
					printf("%d ", incoming_paths[i]);
				}
				printf("\n");
			}
		}

		/* get vertex disjoint paths */
		int* left_checklist = (int*) malloc(sizeof(int) * rows);
		int* right_checklist = (int*) malloc(sizeof(int) * cols);
		for (int _ = 0; _ < rows; ++_) {
			left_checklist[_] = 0;
		}
		for (int _ = 0; _ < cols; ++_) {
			right_checklist[_] = 0;
		}
		for (int i = 0; i < number_of_edges_in_all_paths; i += 2 * global_min_length) {
			if (DEBUG_VERTEX_DISJOINT_ALG && world_rank == 0) {
				printf("probing path at %d \n", i);
			}
			int path_conflicts = 0;
			/* ensure path doesn't conflict */
			for (int j = i; j < i + 2 * global_min_length; j += 2) {
				int left_vertex = incoming_paths[j];
				int right_vertex = incoming_paths[j + 1];
				if (left_checklist[left_vertex] == 1 || right_checklist[right_vertex] == 1) {
					/* conflicting path */
					path_conflicts = 1;
					break;
				}

			}
			if (path_conflicts) {
				continue; 
			}
			if (DEBUG_VERTEX_DISJOINT_ALG && world_rank == 0) {
				printf("accept path at %d \n", i);
			}
			/* apply path */
			for (int j = i; j < i + 2 * global_min_length; j += 2) {
				int left_vertex = incoming_paths[j];
				int right_vertex = incoming_paths[j + 1];
				left_checklist[left_vertex] = 1;
				right_checklist[right_vertex] = 1;
				if (current_matching[left_vertex*cols + right_vertex] == 1) {
					current_matching[left_vertex*cols + right_vertex] = 0;
				} else {
					current_matching[left_vertex*cols + right_vertex] = 1;
				}
			}
		}

		free(vertices_right_matches);
		free(vertices_left_free_buffer);
		free(my_vertices_left_free);
		IntList_dealloc(&augmenting_path_lengths);
		IntList_dealloc(&augmenting_paths);
		IntList_dealloc(&outgoing_paths);
		free(number_paths_per_process);
		free(incoming_paths);
		free(displacements);
		free(left_checklist);
		free(right_checklist);
	}
	
	return current_matching;
}
