#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define handle_cr_error(msg, err_val) \
	{ fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE); }

int main(int argc, char **argv)
{
	int ret_val;

	if(argc != 2){
		printf("Incorrect input! Need 1 argument: the limit of summation.\n");
		exit(EXIT_FAILURE);
	}

	// the limit of summation
	int global_limit;
	global_limit = atoi(argv[1]);

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

	// determine the task
	int lower_lim=0;
	int upper_lim=0;
	if(rank < global_limit){
		int val_per_proc, val_left;
		val_per_proc = global_limit/num_procs;
		val_left = global_limit%num_procs;
		if(val_left != 0){
			if(rank < val_left){
				val_per_proc++;
				lower_lim = rank*val_per_proc + 1;
				upper_lim = lower_lim + val_per_proc - 1;
			}
			else{
				lower_lim = val_per_proc*rank + val_left + 1;
				upper_lim = lower_lim + val_per_proc - 1;
			}
		}
		else{
			lower_lim = val_per_proc*rank + 1;
			upper_lim = lower_lim + val_per_proc - 1;
		}
	}
	
	// start calculations
	double local_result = 0;
	for(double i = lower_lim; i <= upper_lim; i++)
		local_result += 1/i;

	// send/collect results
	if(rank == 0){
		MPI_Status status;
		double result = local_result;
		double buf;
		for(int i = 1; i<num_procs; i++){
			ret_val = MPI_Recv(&buf, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(ret_val)
				handle_cr_error("Error running MPI_Recv", ret_val);
			result += buf;
		}
		printf("Result: %lf\n", result);
	}
	else{
		ret_val = MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		if(ret_val)
			handle_cr_error("Error running MPI_Send", ret_val);
	}


	// finalize the MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finalize", ret_val);
	
	return 0;
}