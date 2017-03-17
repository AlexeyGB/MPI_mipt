#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define handle_cr_error(msg, err_val) \
	{fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE);}

int main(int argc, char **argv){
	int ret_val;

	if(argc != 2){
		printf("Need one argument: the number of tests\n");
		exit(EXIT_FAILURE);
	}

	// initialize MPI environment
	ret_val = MPI_Init(&argc, &argv);
	if(ret_val)
		handle_cr_error("Error running MPI_Init", ret_val);

	int num_proc, rank;
	ret_val = MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	if(ret_val)
		handle_cr_error("Error running MPI_Comm_size", ret_val);
	ret_val = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(num_proc != 2){
		printf("Number of processors must be 2!\n");
		exit(EXIT_FAILURE);
	}

	int num_tests;
	num_tests = atoi(argv[1]);
	
	char buf = 'a';
	MPI_Status status;
	if(rank == 0){
		double start_time, end_time;
		double aver_delay=0;
		double *times;
		times = (double *) calloc(num_tests, sizeof(double));

		printf("Step\t\tDelay\n");
		for(int i=0; i<num_tests; i++){
			start_time = MPI_Wtime();
			MPI_Send(&buf, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
			MPI_Recv(&buf, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
			end_time = MPI_Wtime();
			aver_delay += end_time - start_time;
			times[i] = end_time - start_time;
			printf("%d\t\t%.15lf\n", i, end_time-start_time);
		}
		printf("\n\n");
		aver_delay = aver_delay/num_tests;
		printf("Average delay: %.15lf\n\n", aver_delay);

		free(times);
	}
	if(rank == 1){
		for(int i=0; i<num_tests; i++)
		{
			MPI_Recv(&buf, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
			MPI_Send(&buf, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		}
	}

	// finalize the MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finalize", ret_val);
	



	exit(EXIT_SUCCESS);
}