#ifndef LIFE_LIB
#define LIFE_LIB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>

struct game_field{
	int size;
	int **cells;
};

int alloc_field_cells(struct game_field *field, int size);

int free_field_cells(struct game_field *field);

int swap_states(struct game_field *state1,
								 struct game_field *state2);

int get_initial_state(struct game_field *init_state, int getting_way);

int show_state(struct game_field *state, int output_mode);

int calculate_next_state(struct game_field *current_state,
													struct game_field *future_state);


#endif
