#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define A 1 // coeff by du/dx

#define handle_cr_error(msg, err_val) \
	{fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE);}

void determ_task(int *lower_lim, int *upper_lim, int global_lim, int num_procs, int rank);
/* determine task for proc */

double func(double t, double x){
	// function f(t,x) 
	return cos(x);
}

double u_t_0(double t){
	// function u|x=0
	return log(1+t*t);
}

double u_x_0(double x){
	// function u|t=0
	return log(1+x*x)+sin(x);
}

int main(int argc, char **argv){
	// check input args
	if(argc != 6){
		printf("Error! Need 5 args: steps amount: time and x, upper limits for time and x, output file's name (1 if stdout)\n");
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
	int t_max, x_max;				// upper limits 
	char *filename;
	t_steps_n = atoi(argv[1]);
	x_steps_n = atoi(argv[2]);
	t_max = atoi(argv[3]);
	x_max = atoi(argv[4]);
	filename = argv[5];

	// step
	double t_step = ((double)t_max)/(t_steps_n);
	double x_step = ((double)x_max)/(x_steps_n-1);
	
	// alloc memory for data
	double *t_0, *u_prev, *u_curr, *u_next;
	t_0 = (double *) calloc(t_steps_n, sizeof(double));      // u(t,x=0)
	u_prev = (double *) calloc(x_steps_n, sizeof(double));
	u_curr = (double *) calloc(x_steps_n, sizeof(double));
	u_next = (double *) calloc(x_steps_n, sizeof(double));

	// calculate initial and boundary conditions
	for(int step_num=0; step_num <= t_steps_n; step_num++)
		t_0[step_num] = u_t_0(t_step * step_num);
	for(int step_num=0; step_num < x_steps_n; step_num++){
		u_curr[step_num] = u_x_0(x_step * step_num);
		u_prev[step_num] = u_curr[step_num];
	}
	
	// determine task
	int x_first_step, x_last_step;
	determ_task(&x_first_step, &x_last_step, x_steps_n, num_procs, rank);

	//start calculations
	double *tmp;
	for(int t_step_num=1; t_step_num <= t_steps_n; t_step_num++){
		// one step
		for(int x_step_num=x_first_step; x_step_num < x_last_step; x_step_num++){
			if(x_step_num == 0)
				u_next[0] = t_0[t_step_num];
			else if(x_step_num == x_steps_n-1)
				u_next[x_step_num] = u_prev[x_step_num] + 2*t_step*func(t_step*t_step_num, x_step*x_step_num) + 
									 A*t_step/x_step*u_curr[x_step_num-1];
			else
				u_next[x_step_num] = u_prev[x_step_num] + 2*t_step*func(t_step*t_step_num, x_step*x_step_num) + 
									 A*t_step/x_step*(u_curr[x_step_num-1]- u_curr[x_step_num+1]);
		}
		tmp = u_prev;
		u_prev = u_curr;
		u_curr = u_next;
		u_next = tmp;

		// send&recv to/from neirghbors
		if(rank%2 == 0){
			// even
			if(rank != num_procs-1){
				MPI_Recv(u_curr+x_last_step, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &status);
				MPI_Send(u_curr+x_last_step-1, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
			}
			if(rank != 0){
				MPI_Send(u_curr+x_first_step, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
				MPI_Recv(u_curr+x_first_step-1, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &status);
			}
		}
		else{
			// odd
			MPI_Send(u_curr+x_first_step, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
			MPI_Recv(u_curr+x_first_step-1, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &status);
			if(rank != num_procs-1){
				MPI_Recv(u_curr+x_last_step, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &status);
				MPI_Send(u_curr+x_last_step-1, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
			}
		}
	}

	// collect data
	int msg_max_size=x_last_step-x_first_step;
	if(rank == 0){
		// main process => recieve others' results
		double *buf;
		buf = (double *) calloc(msg_max_size, sizeof(double));

		int edge_rank = x_steps_n%num_procs-1;
		int msg_real_size;
		int sender_rank;
		int offset;

		for(int i=1; i<num_procs; i++){
			MPI_Recv(buf, msg_max_size, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &msg_real_size);
			sender_rank = status.MPI_SOURCE;
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
		MPI_Send(u_curr+x_first_step, msg_max_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}

	// print results
	if(rank == 0){
		if(*argv[5] == '1'){
			for(int i=0; i<x_steps_n; i++)
				printf("%lf\t", u_curr[i]);
			printf("\n");
		}
		else{
			FILE *output_file = fopen(filename, "w+");
			for(int i=0; i<x_steps_n; i++)
				fprintf(output_file, "%lf\t", u_curr[i]);
			printf("\n");
			fclose(output_file);
		}
	}

	// free allocated memory
	free(t_0);
	free(u_next);
	free(u_curr);
	free(u_prev);

	// finilize th MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finilize", ret_val);

	exit(EXIT_SUCCESS);
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
