#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <mpi.h>
#include "lib/life.h"

int main(int argc, char **argv){
	int ret_val;
  // parallel mode

  // initialize the MPI environment
  ret_val = MPI_Init(&argc, &argv);
  assert(!ret_val);

  // get the number of processes and rank
  int num_procs, rank;
  ret_val = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  assert(!ret_val);
  ret_val = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  assert(!ret_val);

  if(rank == 0)
	 printf("Welcome to the Conway's Game of Life!\n");

  // size of the field
  int field_size;
  if(rank == 0)
    field_size = get_field_size();
  MPI_Bcast(&field_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // creating two fields and alloc memory
  struct game_field current_state, future_state;
  alloc_field_cells(&current_state, field_size);
  alloc_field_cells(&future_state, field_size);

  // initialize current field
  if(rank == 0)
    get_initial_state(&current_state);
  for(int i=0; i<field_size; i++)
    MPI_Bcast(&current_state.cells[i][0], field_size, MPI_INT, 0, MPI_COMM_WORLD);

  // get output file (choose output mode)
  FILE *output_file = stdout;
  if(rank == 0){
    output_file = get_output_file();
		// show initial field
		if(output_file == stdout)
			show_state(&current_state, output_file);
	}

  // the number of steps
  int steps_num;
  if(rank == 0){
    printf("In this mode you should type the number of steps to be calculated. Then you'll see the result.\n");
    printf("The number of steps: ");
		fflush(stdout);
    scanf("%d", &steps_num);
  }
  MPI_Bcast(&steps_num, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// start time counting
	double start_time, finish_time;
  start_time = MPI_Wtime();

	// determ task
  int lower_lim, upper_lim;
  determ_task(&lower_lim, &upper_lim, field_size, num_procs, rank);

  // start doing job
  MPI_Status status;
  for(int i=0; i<steps_num; i++){

    // calculate
    calculate_next_state_parallel(&current_state, &future_state, lower_lim, upper_lim);
    swap_states(&current_state, &future_state);

		// send&recv to/from neirghbors
    if(rank%2 == 0){
      // even
      if(rank != num_procs-1){
        MPI_Send(&current_state.cells[upper_lim-1][0], field_size, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        MPI_Recv(&current_state.cells[upper_lim][0], field_size, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
      }
      if(rank != 0){
        MPI_Recv(&current_state.cells[lower_lim-1][0], field_size, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
        MPI_Send(&current_state.cells[lower_lim][0], field_size, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      }
    }
    else{
      // odd
      MPI_Recv(&current_state.cells[lower_lim-1][0], field_size, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
      MPI_Send(&current_state.cells[lower_lim][0], field_size, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      if(rank != num_procs-1){
        MPI_Send(&current_state.cells[upper_lim-1][0], field_size, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        MPI_Recv(&current_state.cells[upper_lim][0], field_size, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
      }
    }
    if( (rank == 0) && (num_procs != 1) ){
      MPI_Recv(&current_state.cells[field_size-1][0], field_size, MPI_INT, num_procs-1, 0, MPI_COMM_WORLD, &status);
      MPI_Send(&current_state.cells[0][0], field_size, MPI_INT, num_procs-1, 0, MPI_COMM_WORLD);
    }
    if( (rank == num_procs-1) && (num_procs != 1) ){
      MPI_Send(&current_state.cells[field_size-1][0], field_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
      MPI_Recv(&current_state.cells[0][0], field_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
  }

  // collect the results
  int buf[field_size+1];
  int my_cells_num = upper_lim-lower_lim;
  if(num_procs != 1){
    if(rank == 0){
      for(int i=0; i<field_size-my_cells_num; i++){
        MPI_Recv(buf, field_size+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        for(int j=0; j<field_size; j++)
          current_state.cells[buf[0]][j] = buf[j+1];
      }
    }
    else{
      for(int i=lower_lim; i<upper_lim; i++){
        buf[0] = i;
        for(int j=0; j<field_size; j++)
          buf[j+1] = current_state.cells[i][j];
        MPI_Send(buf, field_size+1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      }
    }
  }

  finish_time = MPI_Wtime();

  // show the result
  if(rank == 0){
    show_state(&current_state, output_file);
    printf("\nTime: %.15lf\n\n", finish_time-start_time);
  }

  // finalize the MPI environment
  MPI_Finalize();

  return 0;
}
