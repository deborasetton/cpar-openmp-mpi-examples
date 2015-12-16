/*
 * Version of parallel pi calculation that does not incur in the false sharing
 * problem of the previous version.
 *
 * Example from: Introduction to OpenMP - Tim Mattson (Intel), video 08.
 * https://www.youtube.com/playlist?list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG
 */

#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 8

static long num_steps = 499999999;
double step;

/**
 * Entry point.
 */
int main(void) {
  int i, nthreads;
  double pi = 0.0;

  step = 1.0 / (double)num_steps;

  omp_set_num_threads(NUM_THREADS);

  #pragma omp parallel
  {
    // These variables are private. They go away after the block finishes.
    int i, id, nthrds;
    double x, local_sum = 0.0;

    id = omp_get_thread_num();
    printf("[%d] Hello!\n", id);

    nthrds = omp_get_num_threads();
    if (id == 0) {
      nthreads = nthrds;
    }

    /* Cyclic distribution of work for each thread. */
    for (i = id; i < num_steps; i += nthrds) {
      x = (i + 0.5) * step;
      local_sum += 4.0 / (1.0 + x * x);
    }

    #pragma omp critical
    {
      pi += local_sum * step;
    }

    // Alternative using omp atomic:
    // local_sum = local_sum * step;
    // #pragma omp atomic
    // pi += sum;

  }

  printf("Pi is: %f\n", pi);
  return 0;
}
