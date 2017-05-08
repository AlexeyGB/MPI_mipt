#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>
//#include <mpi.h>
#include "lib/life.h"

int main(int argc, char **argv){
	int ret_val;
	printf("Welcome to the Conway's Game of Life!\n");
	// step by step mode

	// size of the field
	int field_size;
	field_size = get_field_size();

	// creating two fields and alloc memory
	struct game_field current_state, future_state;
	alloc_field_cells(&current_state, field_size);
	alloc_field_cells(&future_state, field_size);

	// initialize current field
	get_initial_state(&current_state);

	// get output file (choose output mode)
	FILE *output_file;
	output_file = get_output_file();

	// start doing job
	printf("Step by step mode.\nYou should type the number of steps to be calculated before the next showed state. To finish the program type 0.\n");
	show_state(&current_state, output_file);

	int steps_before_show;
	printf("Steps number: ");
	scanf("%d", &steps_before_show);
	while(steps_before_show>0){
		for(int i=0; i<steps_before_show; i++){
			calculate_next_state(&current_state, &future_state);
			swap_states(&current_state, &future_state);
		}
		show_state(&current_state, output_file);
		printf("Steps number: ");
		scanf("%d", &steps_before_show);
	}


	fclose(output_file);
	printf("Program finished\n");
	free_field_cells(&current_state);
	free_field_cells(&future_state);

	return 0;
}
