#include <stdio.h>
#include <stdlib.h>

#include "Queue.h"
#include "IntList.h"

#define DEBUG_BFS_FOREST 0
#define DEBUG_DFS 0

int* hopcroftKarpSequential(int* graph, int rows, int cols) {
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

		int* vertices_left_free_table = (int*) malloc(sizeof(int) * rows);
		for (int i = 0; i < rows; ++i) {
			vertices_left_free_table[i] = 0;
		}

		/* put all left free vertices in vertices_left_free */
		IntList vertices_left_free;
		IntList_init(&vertices_left_free);
		for (int i = 0; i < rows; ++i) {
			int empty_row = 1;
			for (int j = 0; j < cols; ++j) {
				if (current_matching[i*cols + j] == 1) {
					empty_row = 0;
					break;
				}
			}
			if (empty_row == 1) {
				IntList_appendToEnd(&vertices_left_free, i);
				/* additionally initialize vertices_left_free_table */
				vertices_left_free_table[i] = 1;
			}
		}

		/* build MS-BFS forest */
		IntList vertices_right_free;
		IntList_init(&vertices_right_free);
		int* explored_right = (int*) malloc(sizeof(int) * cols);
		IntList* parent_left = (IntList*) malloc(sizeof(IntList) * rows); /* [left_vertex] => right_vertices */
		for (int i = 0; i < rows; ++i) {
			IntList_init(&parent_left[i]);
		}
		IntList* parent_right = (IntList*) malloc(sizeof(IntList) * cols); /* [right_vertex] => left_vertices */
		for (int i = 0; i < cols; ++i) {
			IntList_init(&parent_right[i]);
			/* wow efficient */
			explored_right[i] = 0;
		}
		int stop_bfs = 0;
		Queue my_queue;
		Queue_init(&my_queue);
		for (int i = 0; i < IntList_getLength(&vertices_left_free); ++i) {
			Queue_enqueue(&my_queue, vertices_left_free.arr[i]);
		}
		while (!Queue_isEmpty(&my_queue)) {
			int current_vertex = Queue_dequeue(&my_queue);
			if (DEBUG_BFS_FOREST) {
				printf("visiting l %d \n", current_vertex);
			}
			for (int j = 0; j < cols; ++j) {
				if (graph[current_vertex*cols + j]) {
					/* parent */
					IntList_appendToEnd(&parent_right[j], current_vertex);
					if (DEBUG_BFS_FOREST) {
						printf("drop by r %d \n", j);
					}
					if (!explored_right[j]) {
						explored_right[j] = 1;
						if (vertices_right_matches[j] == -1) {
							stop_bfs = 1;
							IntList_appendToEnd(&vertices_right_free, j);
						} else {
							IntList_appendToEnd(&parent_left[vertices_right_matches[j]], j); /* I had these two swapped */
							if (!stop_bfs) {
								Queue_enqueue(&my_queue, vertices_right_matches[j]);
							}
						}
						
					}
				}
			}
		}

		if (DEBUG_BFS_FOREST) {
			printf("vertices_right_free: \n");
			IntList_print(&vertices_right_free, 0);
		}
		
		/* initializing for dfs */
		int* visited_left = (int*) malloc(sizeof(int) * rows);
		for (int i = 0; i < rows; ++i) {
			visited_left[i] = 0;
		}
		int* dfs_parent_left = (int*) malloc(sizeof(int) * rows);
		int* dfs_parent_right = (int*) malloc(sizeof(int) * cols);
		for (int i = 0; i < rows; ++i) {
			dfs_parent_left[i] = -1;
		}
		for (int i = 0; i < cols; ++i) {
			dfs_parent_right[i] = -1;
		}

		if (IntList_getLength(&vertices_right_free) == 0) {
			break;
		}

		/* get vertex disjoint a.p.s using SS-DFS */
		if (DEBUG_DFS) {
			printf("DFS BEGINS \n");
		}
		for (int i = 0; i < IntList_getLength(&vertices_right_free); ++i) {
			IntList stack;
			IntList_init(&stack);
			IntList_appendToEnd(&stack, vertices_right_free.arr[i]);
			while (!IntList_isEmpty(&stack)) {
				int right_vertex = IntList_pop(&stack);
				if (DEBUG_DFS) {printf("r %d \n", right_vertex);}
				int path_found = 0;
				/* for every parent of this right vertex */
				for (int i = 0; i < IntList_getLength(&parent_right[right_vertex]); ++i) {
					int left_vertex = parent_right[right_vertex].arr[i];
					if (!visited_left[left_vertex]) {
						if (DEBUG_DFS) {
							printf("l %d \n", left_vertex);
						}
						visited_left[left_vertex] = 1;
						dfs_parent_left[left_vertex] = right_vertex;
						if (vertices_left_free_table[left_vertex]) {
							/* left vertex is free */
							if (DEBUG_DFS) {printf("left v free \n");}
							int left_cursor = left_vertex;
							int right_cursor = right_vertex;
							if (DEBUG_DFS) {printf("BEGIN AP \n");}
							if (DEBUG_DFS) {printf("%d -> %d \n", left_cursor, right_cursor);}
							current_matching[left_cursor * cols + right_cursor] = !current_matching[left_cursor * cols + right_cursor];
							while (dfs_parent_right[right_cursor] != -1) {
								left_cursor = dfs_parent_right[right_cursor];
								if (DEBUG_DFS) {printf("%d -> %d \n", left_cursor, right_cursor);}
								current_matching[left_cursor * cols + right_cursor] = !current_matching[left_cursor * cols + right_cursor];
								right_cursor = dfs_parent_left[left_cursor];
								if (DEBUG_DFS) {printf("%d -> %d \n", left_cursor, right_cursor);}
								current_matching[left_cursor * cols + right_cursor] = !current_matching[left_cursor * cols + right_cursor];
							}
							if (DEBUG_DFS) {printf("END OF AP \n");}
							path_found = 1;
							break;
						} else {
							/* left vertex is matched */
							if (DEBUG_DFS) {printf("left v matched \n");}
							int next_right_vertex = parent_left[left_vertex].arr[0];
							dfs_parent_right[next_right_vertex] = left_vertex;
							if (DEBUG_DFS) {printf("adding %d to stack \n", next_right_vertex);}
							IntList_appendToEnd(&stack, next_right_vertex); // there is only 1 parent to a matched left vertex
						}
					}
				}
				if (path_found) {
					break;
				}
			}
			IntList_dealloc(&stack);
		}
		if (DEBUG_DFS) {
			printf("DFS ENDS \n");
		}
		
		/* */
		for (int i = 0; i < rows; ++i) {
			IntList_dealloc(&parent_left[i]);
		}
		free(parent_left);
		for (int i = 0; i < cols; ++i) {
			IntList_dealloc(&parent_right[i]);
		}
		free(parent_right);
		/* */
		
		free(explored_right);
		free(vertices_right_matches);
		free(vertices_left_free_table);
		Queue_dealloc(&my_queue);
		IntList_dealloc(&vertices_left_free);
		IntList_dealloc(&vertices_right_free);
		free(visited_left);
		free(dfs_parent_left);
		free(dfs_parent_right);
	}
	
	return current_matching;
}