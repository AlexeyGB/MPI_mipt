#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>


#define handle_cr_error(msg, err_val) \
    { fprintf(stderr, "%s\nError value %d\n", msg, err_val); exit(EXIT_FAILURE); } 

int main(int argc, char **argv)
{
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


    if(rank == 0){
    	int value = 0;

   		// print value
    	printf("rank %d, value %d\n", rank, value);
    	
    	if(num_procs != 1){
			// send value to the proc 1
			ret_val = MPI_Send(&value, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
			if(ret_val)
				handle_cr_error("Error running MPI_Send", ret_val);

			// recieve value form the last proc
			MPI_Status status;
			ret_val = MPI_Recv(&value, 1, MPI_INT, num_procs-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(ret_val)
				handle_cr_error("Error running MPI_Recv", ret_val);

			// print new value
			value++;
			printf("rank %d, value %d\n", rank, value);
		}
	}    
	else{
		int value;

		// recv value from the previous proc
		MPI_Status status;
		ret_val = MPI_Recv(&value, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(ret_val)
			handle_cr_error("Error running MPI_Recv", ret_val);

		// print new value
		value++;
		printf("rank %d, value %d\n", rank, value);

		// sent new value to the next proc
		if(rank != (num_procs - 1))
			ret_val = MPI_Send(&value, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
		else
			ret_val = MPI_Send(&value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		if(ret_val)
			handle_cr_error("Error running MPI_Send", ret_val);
	}

	// finalize the MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finalize", ret_val);
	
	return 0;
}