#include <omp.h>
#include <stdio.h>
#include <unistd.h>

void foo(int id) {
  if (id == 1) {
    sleep(3);
  }
  printf("[%d] Completed foo\n", id);
}

void bar(int id) {
  printf("[%d] Completed bar\n", id);
}

int main(void) {
  double elapsed = 0.0;
  elapsed -= omp_get_wtime();

  #pragma omp parallel
  {
    int my_id = omp_get_thread_num();

    #pragma omp task
    foo(my_id);
    #pragma omp barrier
    #pragma omp single
    {
      #pragma omp task
      bar(my_id);
    }
  }

  elapsed += omp_get_wtime();
  printf("Completed in %lf\n", elapsed);
}
