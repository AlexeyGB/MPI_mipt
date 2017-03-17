#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <gmp.h>

#define MAX_RECV_SIZE 1024
#define DEFAULT_PREC 1024

#define handle_cr_error(msg, err_val) \
	{ fprintf(stderr, "%s\nError code %d\n",msg, err_val); exit(EXIT_FAILURE); }

void mpf_factorial(mpf_t rop, unsigned long int n);

int main(int argc, char **argv){
	// check args
	if(argc != 2){
		printf("Incorrect input! Need 1 argument: the limit of summation\n");
		exit(EXIT_FAILURE);
	}

	// set mpf_t default precision
	mpf_set_default_prec(DEFAULT_PREC);

	// get args
	unsigned long int  global_limit;
	if(argc == 2)
		global_limit = atol(argv[1]);
	
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
				lower_lim = rank*val_per_proc;
				upper_lim = lower_lim + val_per_proc - 1;
			}
			else{
				lower_lim = val_per_proc*rank + val_left;
				upper_lim = lower_lim + val_per_proc - 1;
			}
		}
		else{
			lower_lim = val_per_proc*rank;
			upper_lim = lower_lim + val_per_proc - 1;
		}
	}
	
	// start calculations
	mpf_t local_result;
	mpf_init(local_result);
	mpf_t factorial;
	mpf_init(factorial);
	mpf_t mid_result;
	mpf_init(mid_result);
	
	for(unsigned long int current_step=lower_lim; current_step <= upper_lim; current_step++){
		mpf_set_ui(mid_result, 1);
		// calc actual factorial
		if(current_step == lower_lim){
			//s_fact_time = clock();
			mpf_factorial(factorial, current_step);
			//f_fact_time=clock();
		}
		else
			mpf_mul_ui(factorial, factorial, current_step);
			
		// calc middle result
		mpf_div(mid_result, mid_result, factorial);
		mpf_add(local_result, local_result, mid_result);
	}	
	
	// send/collect results
	if(rank == 0){
		MPI_Status status;
 
		char recv_str[MAX_RECV_SIZE];
		for(int i = 1; i<num_procs; i++){
			ret_val = MPI_Recv(&recv_str, MAX_RECV_SIZE, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(ret_val)
				handle_cr_error("Error running MPI_Recv", ret_val);
			
			mpf_set_str(mid_result, recv_str, 10);
			mpf_add(local_result, local_result, mid_result);
		}
	}
	else{
		mp_exp_t exp;
		char *str;
		str = mpf_get_str(NULL, &exp, 10, 0, local_result);
		
		char *str_to_send;
		long str_len = strlen(str);
		long str_to_send_len = str_len+13;
		str_to_send = (char *) calloc(str_to_send_len, sizeof(char));
		memcpy(str_to_send, str, sizeof(char)*str_len);
		str_to_send[str_len] = 'e';
		sprintf(str_to_send+str_len+1, "%ld", exp);

		ret_val = MPI_Send(&str_to_send, strlen(str_to_send)+1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		if(ret_val)
			handle_cr_error("Error running MPI_Send", ret_val);

		free(str_to_send);
		free(str);
	}

	// timer
	end_time = MPI_Wtime();

	// print results
	if(rank == 0){
		printf("Time: %lfs\n\n", end_time-start_time);
		printf("Result: ");
		mpf_out_str(stdout, 10, 0, local_result);
		printf("\n\n");
	}

	// free GMP memory
	mpf_clear(local_result);
	mpf_clear(mid_result);
	mpf_clear(factorial);

	// finalize the MPI environment
	ret_val = MPI_Finalize();
	if(ret_val)
		handle_cr_error("Error running MPI_Finalize", ret_val);
	
	return 0;
}

void mpf_factorial(mpf_t rop, unsigned long int n){
	mpz_t factorial;
	mpz_init(factorial);
	mpz_fac_ui(factorial, n);
	mpf_set_z(rop, factorial);
	mpz_clear(factorial);
}