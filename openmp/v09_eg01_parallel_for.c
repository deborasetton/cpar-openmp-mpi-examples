/**
 * Example of #pragma omp for.
 */

#include <omp.h>
#include <stdio.h>

const int N = 50;

int main(void)
{
  omp_set_num_threads(4);

  #pragma omp parallel
  {
    int my_rank = omp_get_thread_num();

    // Syntax note: no opening '{' allowed after this pragma.
    // OpenMP makes the loop control index on a parallel loop private to a
    // thread.
    #pragma omp for
    for (int i = 0; i < N; ++i) {
      printf("[%d] i = %d\n", my_rank, i);
    }
  }

  printf("Done!\n");
  return 0;
}

// Schedule modes:
// ---------------
//
// Schedule at compile time:
// schedule(static[,chunk])
//
// Schedule at run time:
// schedule(dynamic[,chunk=1])
//
// Dynamic, blocks get smaller over time.
// schedule(guided[,chunk])
//
// Pass values at runtime.
// schedule(runtime)
//
// Compiler decides.
// schedule(auto)
