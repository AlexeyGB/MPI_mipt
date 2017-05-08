#include "life.h"

int alloc_field_cells(struct game_field *field, int size){
	field->size = size;
	field->step = 0;
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
	assert(state1->size == state2->size);

	int **tmp_cells;
	tmp_cells = state1->cells;
	state1->cells = state2->cells;
	state2->cells = tmp_cells;

	int tmp_step;
	tmp_step = state1->step;
	state1->step = state2->step;
	state2->step = tmp_step;
	return 0;
}

int get_initial_state(struct game_field *init_state){
	/*
		several ways to get initial field
		1. from file
		- randomly
		- ...
	*/
	int way_to_init_field;
	printf("Ways of getting the initial state of the field:\n");
	printf("1. from file\n");
	printf("2. randomly\n");
	printf("Your choice: ");
	fflush(stdout);
	scanf("%d", &way_to_init_field);
	while( (way_to_init_field != 1) && (way_to_init_field != 2) ){
		printf("Incorrect!\n\n");
		printf("Ways of getting the initial state of the field:\n");
		printf("1. from file\n");
		printf("2. randomly\n");
		printf("Your choice: ");
		fflush(stdout);
    scanf("%d", &way_to_init_field);
	}
	printf("\n");

	int field_size = init_state->size;
	init_state->step = 0;
	char input_file_name[64];
	switch (way_to_init_field) {
		case 1:
			printf("Input file name: ");
			fflush(stdout);
	    scanf("%s", input_file_name);
			FILE *input_file = fopen(input_file_name, "r");
			assert(input_file);
			for(int i=0; i<field_size; i++)
				for(int j=0; j<field_size; j++)
					fscanf(input_file, "%d", &init_state->cells[i][j]);
			fclose(input_file);
			break;
		case 2:
			printf("The persantage of 1s: ");
			fflush(stdout);
			double frontier;
			int rand_val;
			scanf("%lf", &frontier);
			frontier = RAND_MAX*(1-frontier);
			srand(time(NULL));
			for(int i=0; i<field_size; i++)
				for(int j=0; j<field_size; j++){
					rand_val = rand();
					if(rand_val < frontier)
						init_state->cells[i][j] = 0;
					else
						init_state->cells[i][j] = 1;
				}
			break;
	}
	return 0;
}

int show_state(struct game_field *field, FILE *output_file){
	/*
		prints the field
		possible ways:
		1. to stdout
		2. to file
	*/
	int field_size = field->size;

	fprintf(output_file, "\n");
  fprintf(output_file, "Step %d\n", field->step);
	fprintf(output_file, "\n");
	for(int i=0; i<field_size; i++){
		for(int j=0; j<field_size; j++){
		  fprintf(output_file, "%d ", field->cells[i][j]);
		}
		fprintf(output_file, "\n");
	}
  fprintf(output_file, "\n");
	fflush(output_file);
	return 0;
}

int calculate_next_state(struct game_field *current_state,
												 struct game_field *future_state){
	assert(current_state->size == future_state->size);

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
	future_state->step = current_state->step + 1;

	return 0;
}

int calculate_next_state_parallel(struct game_field *current_state,
												 struct game_field *future_state, int lower_lim, int upper_lim){
	assert(current_state->size == future_state->size);

	int field_size = current_state->size;
	int neighbours_alive;
	for(int i=lower_lim; i<upper_lim; i++)
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
	future_state->step = current_state->step + 1;

	return 0;
}

void determ_task(int *lower_lim, int *upper_lim, int global_lim, int num_procs, int rank){
	int val_per_proc, val_left;
	val_per_proc = global_lim/num_procs;
	val_left = global_lim%num_procs;
	if(val_left != 0){
		if(rank < val_left){
			val_per_proc++;
			*lower_lim = rank*val_per_proc;
			*upper_lim = *lower_lim + val_per_proc;
		}
		else{
			*lower_lim = val_per_proc*rank + val_left;
			*upper_lim = *lower_lim + val_per_proc;
		}
	}
	else{
		*lower_lim = val_per_proc*rank;
		*upper_lim = *lower_lim + val_per_proc;
	}
}

FILE *get_output_file(){
	int output_mode;
	printf("Output modes:\n");
	printf("1. to stdout\n");
	printf("2. to file\n");
	printf("Your choice: ");
	fflush(stdout);
	scanf("%d", &output_mode);
	while( (output_mode != 1) && (output_mode != 2) ){
		printf("Incorrect!\n\n");
		printf("Output modes:\n");
		printf("1. to stdout\n");
		printf("2. to file\n");
		printf("Your choice: ");
		fflush(stdout);
    scanf("%d", &output_mode);
	}
	char output_file_name[64];
	FILE *output_file;
	if(output_mode == 2){
		printf("Output file name: ");
		fflush(stdout);
    scanf("%s", output_file_name);
		output_file = fopen(output_file_name, "w+");
		assert(output_file);
	}
	else
		output_file = stdout;
	printf("\n");

	return output_file;
}

int get_field_size(){
	int field_size;
	printf("Choose size of the field: ");
	fflush(stdout);
	scanf("%d", &field_size);
	while(field_size < 3){
		printf("Entered size is too small! Should be at least 3.\n");
		printf("Choose size of the field: ");
		fflush(stdout);
    scanf("%d", &field_size);
	}
	printf("\n");

	return field_size;
}
