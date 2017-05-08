#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>

#define STEPS_PER_TASK 5000

#define handle_cr_error(msg) \
        { fprintf(stderr, "%s\n", msg); perror("Details"); exit(EXIT_FAILURE); }


struct _global_task{
  long double lower_lim;
  long double upper_lim;
  long double result;
  long double precision;
};

long double func(long double x){
  return cos(1/x/x);
}

long double max_f_second_deriv(long double x){
  return (6*x*x+4)/powl(x,6);
}

long double current_step(long double lower_lim, long double precision){
  return pow( precision*12/STEPS_PER_TASK/max_f_second_deriv(lower_lim), 1.0/3.0);
}

void *thread_job(void *global_task);

pthread_mutex_t mutex;

int main(int argc, char **argv){
  if(argc < 3){
    printf("Too few arguments!\nMust be number of threads and required precision.\n");
    exit(EXIT_FAILURE);
  }


  int threads_num = atoi(argv[1]);
  long double precision = atof(argv[2]);

  struct _global_task global_task;
  global_task.lower_lim = 0.1;
  global_task.upper_lim = 1;
  global_task.result = 0;
  global_task.precision = precision;

  // create mutex
  if( pthread_mutex_init(&mutex, NULL) != 0 )
    handle_cr_error("pthread_mutex_init");

  // It seems that using console utility "time" is better :)
  //long double begin_time, end_time;
  //begin_time = (long double)(clock())/CLOCKS_PER_SEC;

  pthread_t * thread_pool;
  thread_pool = (pthread_t *) calloc(threads_num, sizeof(pthread_t ));
  for(int i=0; i<threads_num; i++){
    if( pthread_create(&thread_pool[i], NULL, thread_job, &global_task) != 0 )
      handle_cr_error("pthread_create");
  }


  for(int i=0; i<threads_num; i++){
    if( pthread_join(thread_pool[i], NULL) != 0 )
        handle_cr_error("pthread_join");
  }

  //end_time = (long double)(clock())/CLOCKS_PER_SEC;

  printf("\nResult: %.30Lf\n", global_task.result);
  //printf("Runtime: %.10Lf s\n\n", end_time - begin_time);

  return 0;
}

void *thread_job(void *task){
  struct _global_task *global_task;
  global_task = (struct _global_task *) task;

  long double local_lower_lim = 0;
  long double local_upper_lim = 0;
  long double local_result = 0;
  long double step = 0;
  long double local_segment;
  long double precision = global_task->precision;

  while(global_task->lower_lim != global_task->upper_lim){
    if( pthread_mutex_lock(&mutex) != 0 )
      handle_cr_error("pthread_mutex_lock");

    if(global_task->lower_lim == global_task->upper_lim)
      break;

    local_lower_lim = global_task->lower_lim;
    step = current_step(local_lower_lim, precision);
    local_segment = step*STEPS_PER_TASK;

    if(global_task->upper_lim >= local_lower_lim+local_segment)
      local_upper_lim = local_lower_lim+local_segment;
    else
      local_upper_lim = global_task->upper_lim;
    global_task->lower_lim = local_upper_lim;

    if( pthread_mutex_unlock(&mutex) != 0 )
      handle_cr_error("pthread_mutex_unlock");

    while(local_lower_lim != local_upper_lim){
      if(step > local_upper_lim-local_lower_lim)
        step = local_upper_lim-local_lower_lim;

      local_result += step/2.*(func(local_lower_lim+step)+func(local_lower_lim));
      local_lower_lim += step;
    }
  }

  if( pthread_mutex_lock(&mutex) != 0 )
    handle_cr_error("pthread_mutex_lock");
  global_task->result += local_result;
  if( pthread_mutex_unlock(&mutex) != 0 )
    handle_cr_error("pthread_mutex_unlock");

  pthread_exit(NULL);
}
