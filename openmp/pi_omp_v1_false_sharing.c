/*
 * This is the version of parallel PI that was presented as a solution
 * on https://www.youtube.com/watch?v=OuzYICZUthM.
 *
 * It uses round robin to distribute the work among the threads.
 * It runs into the *false sharing* problem due to the whole sum[NUM_THREADS]
 * array fitting in a single cache line, and so it's constantly
 * invalidated by the other threads, even though the threads are not actually
 * updating the same memory space.
 */

#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 1

static long num_steps = 499999999;
double step;

/**
 * Entry point.
 */
int main(void) {
  int i, nthreads;
  double pi, sum[NUM_THREADS];

  step = 1.0 / (double)num_steps;

  omp_set_num_threads(NUM_THREADS);

  #pragma omp parallel
  {
    // These variables are private. They go away after the block finishes.
    int i, id, nthrds;
    double x;

    id = omp_get_thread_num();
    printf("[%d] Hello!\n", id);

    nthrds = omp_get_num_threads();
    if (id == 0) {
      nthreads = nthrds;
    }

    /* Cyclic distribution of work for each thread. */
    for (i = id, sum[id] = 0.0; i < num_steps; i += nthrds) {
      x = (i + 0.5) * step;
      sum[id] += 4.0 / (1.0 + x * x);
    }
  }

  for (i = 0, pi = 0.0; i < nthreads; ++i) {
    pi += sum[i] * step;
  }

  printf("Pi is: %f\n", pi);

  return 0;

}
