#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define handle_cr_error(msg, err_val) \
	{ fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE); }

double func(double x){
	// function here
	return x;
}
void determ_task(int *lower_lim, int *upper_lim, int global_lim, int num_procs, int rank);


int main(int argc, char **argv){
	// check args
	if(argc != 2){
		printf("Incorrect input! Need 1 argument: step\n");
		exit(EXIT_FAILURE);
	}

	// get arg
	double step;
	step = atof(argv[1]);
	
	int ret_val;
	
	// initialize the MPI environment
	ret_val = MPI_Init(&argc, &argv);
	if(ret_val)
		handle_cr_error("Error running MPI_Init", ret_val);

	// get the number of processes
	int num_procs;
	ret_val = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	if(ret_val)
		handle_cr_error("Error running MPI_Comm_size", ret_val);

	// get the rank of the process
	int rank;
	ret_val = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(ret_val)
		handle_cr_error("Error running MPI_Comm_rank", ret_val);
	
	// timer
	double start_time, end_time;
	start_time = MPI_Wtime();

	// determine task
	int first_step, last_step;
	int global_step_num;
	global_step_num = 1.0/step;
	
	determ_task(&first_step, &last_step, global_step_num, num_procs, rank);
	
	// start calculations 
	double local_result=0;
	
	for(int current_step=first_step; current_step <last_step; current_step++){
		local_result += (func(step*(current_step+1)) + func(step*current_step))/2 * step;
	}

	// send/collect results
	if(rank == 0){
		MPI_Status status;
 		double recv_result;
		for(int i = 1; i<num_procs; i++){
			ret_val = MPI_Recv(&recv_result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(ret_val)
				handle_cr_error("Error running MPI_Recv", ret_val);
			local_result += recv_result;
		}
	}
	else{
		ret_val = MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		if(ret_val)
			handle_cr_error("Error running MPI_Send", ret_val);
	}

	// timer
	end_time = MPI_Wtime();

	// print results
	if(rank == 0){
		printf("Time: %lfs\n\n", end_time-start_time);
		printf("Result: %lf\n\n", local_result);
	}

	// finalize the MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finalize", ret_val);
	
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
