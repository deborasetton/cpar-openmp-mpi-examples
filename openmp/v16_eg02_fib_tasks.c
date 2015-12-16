#include <omp.h>
#include <stdio.h>
#include <unistd.h>       /* syscall */
#include <sys/syscall.h>  /* SYS_gettid */

#define DEBUG 0

int fib(int n) {
  int my_id = omp_get_thread_num();
  int x, y;

  if (DEBUG) {
    printf("\t\t[%d][tid.%d] Calculating fib(%d)\n", my_id,
           (int)syscall(SYS_gettid), n);
  }

  if (n < 2) {
    return n;
  }

  #pragma omp task shared(x)
  x = fib(n-1);
  #pragma omp task shared(y)
  y = fib(n-2);
  #pragma omp taskwait
  return x + y;
}

int main(void) {
  const int N = 30;
  double elapsed = 0.0;
  int i;

  elapsed -= omp_get_wtime();

  #pragma omp parallel
  {
    int my_id = omp_get_thread_num();
    #pragma omp for
    for (i = 1; i <= N; ++i) {
      printf("[%d][tid.%d] fib(%d) = %d\n", my_id,
             (int)syscall(SYS_gettid), i, fib(i));
    }
  }

  elapsed += omp_get_wtime();
  printf("Completed in %lfs\n", elapsed);
}
