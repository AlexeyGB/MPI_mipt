#ifndef LIFE_LIB
#define LIFE_LIB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

struct game_field{
	int size;
	int step;
	int **cells;
};

int alloc_field_cells(struct game_field *field, int size);

int free_field_cells(struct game_field *field);

int swap_states(struct game_field *state1,
								 struct game_field *state2);

int get_initial_state(struct game_field *init_state);

int show_state(struct game_field *state, FILE *output_file);

int calculate_next_state(struct game_field *current_state,
													struct game_field *future_state);

int calculate_next_state_parallel(struct game_field *current_state,
																	struct game_field *future_state, int lower_lim, int upper_lim);

void determ_task(int *lower_lim, int *upper_lim, int global_lim, int num_procs, int rank);

// communications with user

FILE *get_output_file();

int get_field_size();


#endif
