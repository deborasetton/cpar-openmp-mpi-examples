/**
 * Example of using the following runtime routines in OpenMP:
 * - omp_set_dynamic
 * - omp_set_num_threads
 * - omp_get_num_procs
 * - omp_get_thread_num
 * - omp_get_num_threads
 */

#include <omp.h>
#include <stdio.h>

void do_lots_of_stuff(int id);

/**
 * Entry point.
 */
int main(void) {
  int num_threads;

  // Disable dynamic mode.
  omp_set_dynamic(0);

  // Give me, please, one thread per process.
  omp_set_num_threads(omp_get_num_procs());

  #pragma omp parallel
  {
    int id = omp_get_thread_num();

    #pragma omp single
    {
      // How many threads did we _actually_ get?
      num_threads = omp_get_num_threads();
      printf("[%d] We have %d threads working here.\n", id, num_threads);
    }

    do_lots_of_stuff(id);
  }

  return 0;
}

void do_lots_of_stuff(int id) {
  printf("[%d] Doing lots of interesting things!...\n", id);
}
