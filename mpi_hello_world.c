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

	// get the name of the processor
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int proc_name_len;
	ret_val = MPI_Get_processor_name(processor_name, &proc_name_len);

	// print hello world
	printf("Rank %d out of %d, processor: %s\n", rank, num_procs, processor_name);

	// finalize the MPI environment
    ret_val = MPI_Finalize();
    if(ret_val)
        handle_cr_error("Error running MPI_Finalize", ret_val);
    
	return 0;
}