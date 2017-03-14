#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

#define handle_cr_error(msg, err_val) \
	{fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE);}

void determ_task(int *lower_lim, int *upper_lim, int global_lim, num_procs, rank);
/* determine task for proc */

double func(int t, int x){
	// function f(t,x) here
	return 0;
}

int main(int argc, char **argv){
	// check input args
	if(argc != 4){
		printf("Error! Need 3 args: steps amount: time and x, input file's name\n");
		exit(EXIT_FAILURE);
	} 

	int ret_val;

	// initialize the MPI environment
	ret_val = MPI_Init(&argc, &argv);
	if(ret_val)
		handle_cr_error("Error running MPI_Init", ret_val);

	MPI_Status status;
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

	// read args
	int t_steps_n, x_steps_n; 		// number of steps
	char *filename;
	t_steps_n = atoi(argv[1]);
	x_steps_n = atoi(argv[2]);		
	filename = argv[3];

	// alloc memory for data
	double *t_0, *u_prev, *u_curr, *u_next;
	t_0 = (double *) calloc(t_steps_n, sizeof(double));      // u(t,x=0)
	u_prev = (double *) calloc(x_steps_n, sizeof(double));
	u_curr = (double *) calloc(x_steps_n, sizeof(double));
	u_next = (double *) calloc(x_steps_n, sizeof(double));

	// read input data from file
	FILE *input_file = fopen(filename, 'r');
	for(int i=0; i<t_steps_n, i++)
		fscanf(input_file, "%lf", t_0+i);
	for(int i=0; i<x_steps_n, i++){
		fscanf(input_file, "%lf", u_curr+i);
		u_prev[i] = u_curr[i];
	}

	// determine task
	int x_low_lim, x_up_lim;
	determ_task(&x_low_lim, &x_up_lim, x_steps_n, num_procs, rank);

	//start calculations
	double *tmp;
	for(int t_step=1; t_step<t_steps_n; t_step++){
		// one step
		for(int x_step=x_low_lim; x_step<x_up_lim; x_step++){
			if(x_step == 0)
				u_next[0] = t_0[t_step]; 
			else if(x_step == x_steps_n-1)
				u_next[x_step] = u_prev[x_step]+2*func(t_step, x_step)+(u_curr[x_step-1]);
			else
				u_next[x_step] = u_prev[x_step]+2*func(t_step, x_step)+(u_curr[x_step-1]+u_curr[x_step+1]); 
		}
		tmp = u_prev;
		u_prev = u_curr;
		u_curr = u_next;
		u_next = tmp;
		if(t_step)
		// send&recv to/from neirghbors
		if(rank%2 == 0){
			// even
			if(rank != num_procs-1){
				MPI_Recv(u_curr+x_up_lim, 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				MPI_Send(u_curr+x_up_lim-1, 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD);
			}
			if(rank != 0){
				MPI_Send(u_curr+x_low_lim, 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD);
				MPI_Recv(u_curr+x_low_lim-1, 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			}
		}
		else{
			// odd
			MPI_Send(u_curr+x_low_lim, 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD);
			MPI_Recv(u_curr+x_low_lim-1, 1, MPI_DOUBLE, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(rank != num_procs-1){
				MPI_Recv(u_curr+x_up_lim, 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD);
				MPI_Send(u_curr+x_up_lim-1, 1, MPI_DOUBLE, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			}
		}
	}

	// collect data
	int msg_max_size=x_up_lim-x_low_lim;
	if(rank == 0){
		// main process => recieve others' results
		double *buf;
		buf = (double *) calloc(msg_max_size, sizeof(double));

		int edge_rank = x_steps_n%num_procs-1;
		int msg_real_size;
		int sender_rank;
		int offset;

		for(int i=1; i<num_procs, i++){
			MPI_Recv(buf, msg_max_size, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &msg_real_size);
			sended_rank = status.MPI_SOURCE;
			if((sender_rank > edge_rank) && (edge_rank!=-1))
				offset = (msg_max_size-1) * sender_rank + edge_rank;
			else
				offset = msg_max_size * sender_rank;

			memcpy(u_curr+offset, buf, msg_real_size * sizeof(double));
		}
		free(buf);
	}
	else{
		// slave process => send result to the main proc
		MPI_Send(u_curr+x_low_lim, msg_max_size, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
	}

	// print results
	if(rank == 0){
		for(int i=0; i<x_steps_n; i++)
			printf("%lf ", u_curr[i]);
		printf("\n");
	}

	// free allocated memory
	free(t_0);
	free(u_next);
	free(u_curr);
	free(u_prev);

	// finilize th MPI environment
	ret_val = MPI_Finilize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finilize", ret_val);

	exit(EXIT_SUCCESS);
}

void determ_task(int *lower_lim, int *upper_lim, int global_lim, num_procs, rank){
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