#include "life.h"

int alloc_field_cells(struct game_field *field, int size){
	field->size = size;
	field->cells = (int **) calloc(size, sizeof(int *));
	for(int i=0; i<size; i++){
		field->cells[i] = (int *) calloc(size, sizeof(int));
	}
	return 0;
}

int free_field_cells(struct game_field *field){
	for(int i=0; i<field->size; i++)
		free(field->cells[i]);
	free(field->cells);
	return 0;
}

int swap_states(struct game_field *state1, struct game_field *state2){
	if(state1->size != state2->size)
		exit(EXIT_FAILURE);
	int **tmp;
	tmp = state1->cells;
	state1->cells = state2->cells;
	state2->cells = tmp;
	return 0;
}

int get_initial_state(struct game_field *init_state, int getting_way){
	/*
		several ways to get initial field
		1. from file
		- randomly
		- ...
	*/

	 int field_size = init_state->size;
	 char input_file_name[64];
	 switch (getting_way) {
	 	case 1:
			printf("File name: ");
			scanf("%s", input_file_name);
			FILE *input_file = fopen(input_file_name, "r");
			for(int i=0; i<field_size; i++)
				for(int j=0; j<field_size; j++)
					fscanf(input_file, "%d", &init_state->cells[i][j]);
			fclose(input_file);
			break;
	 }
	 return 0;
}

int show_state(struct game_field *field, int output_mode){
	/*
		prints the field
		possible ways:
		1. to stdout
		- to file
		- ...
	*/
	int field_size = field->size;
	switch (output_mode) {
		case 1:
		  printf("\n");
			for(int i=0; i<field_size; i++){
				for(int j=0; j<field_size; j++)
				  printf("%d ", field->cells[i][j]);
				printf("\n");
			}
			printf("\n");
			break;
	}
	return 0;
}

int calculate_next_state(struct game_field *current_state,
												 struct game_field *future_state){
	if(current_state->size != future_state->size)
		exit(EXIT_FAILURE);

	int field_size = current_state->size;
	int neighbours_alive;
	for(int i=0; i<field_size; i++)
		for(int j=0; j<field_size; j++){
			neighbours_alive = 0;
			neighbours_alive += current_state->cells[ i ][ (j+1)%field_size ];
			neighbours_alive += current_state->cells[ i ][ (j-1+field_size)%field_size ];
			neighbours_alive += current_state->cells[ (i+1)%field_size ][ (j+1)%field_size ];
			neighbours_alive += current_state->cells[ (i+1)%field_size ][ j ];
			neighbours_alive += current_state->cells[ (i+1)%field_size ][ (j-1+field_size)%field_size ];
			neighbours_alive += current_state->cells[ (i-1+field_size)%field_size ][ (j+1)%field_size ];
			neighbours_alive += current_state->cells[ (i-1+field_size)%field_size ][ j ];
			neighbours_alive += current_state->cells[ (i-1+field_size)%field_size ][ (j-1+field_size)%field_size ];

			if(current_state->cells[i][j] == 0){
				if(neighbours_alive == 3)
					future_state->cells[i][j] = 1;
				else
					future_state->cells[i][j] = 0;
			}
			else{
				if( (neighbours_alive < 2) || (neighbours_alive > 3) )
					future_state->cells[i][j] = 0;
				else
					future_state->cells[i][j] = 1;
			}
		}
	return 0;
}
