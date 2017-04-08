#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>
#include "lib/life.h"

int main(int argc, char **argv){
	int game_mode;
	printf("Welcome to the Conway's Game of Life!\n");
	printf("Choose game mode\n");
	printf("1. Step by step\n");
	//printf("2. Speed test\n"); //todo
	printf("Your choice: ");
	scanf("%d", &game_mode);

	if(game_mode == 1){
		// step by step mode

		// size of the field
		int field_size;
		printf("Choose size of the field: ");
		scanf("%d", &field_size);
		while(field_size < 3){
			printf("Entered size is too small! Should be at least 3.\n");
			printf("Choose size of the field: ");
			scanf("%d", &field_size);
		}

		// creating two fields and alloc memory
		struct game_field current_state, future_state;
		alloc_field_cells(&current_state, field_size);
		alloc_field_cells(&future_state, field_size);

		// initialize current field
		int way_to_init_field;
		printf("Ways of getting the initial state of the field:\n");
		printf("1. from file\n");
		printf("Your choice: ");
		scanf("%d", &way_to_init_field);
		get_initial_state(&current_state, way_to_init_field);

		// output mode
		int output_mode;
		printf("Output modes:\n");
		printf("1. to stdout\n");
		printf("Your choice: ");
		scanf("%d", &output_mode);

		// start doing job
		printf("In this mode you should type the number of steps to be calculated before the next showed state. To finish the program type 0.\n");
		int steps_before_show;
		printf("Steps number: ");
		scanf("%d", &steps_before_show);
		while(steps_before_show>0){
			for(int i=0; i<steps_before_show; i++){
				calculate_next_state(&current_state, &future_state);
				swap_states(&current_state, &future_state);
			}
			show_state(&current_state, output_mode);
			printf("Steps number: ");
			scanf("%d", &steps_before_show);
		}

		printf("Program finished\n");
		free_field_cells(&current_state);
		free_field_cells(&future_state);
	}

}
