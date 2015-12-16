#include <omp.h>
#include <stdio.h>

#define N 100

// Each thread will have its own copy of counter, but it will be global to
// all the functions that have visibility to the variable.
int counter = 0;
#pragma omp threadprivate(counter)

int increment() {
  int id = omp_get_thread_num();
  counter++;
  printf("[%d] counter = %d\n", id, counter);
}

int main(void) {
  #pragma omp parallel
  {
    for (int i = 0; i < N; ++i) {
      increment();
    }
  }
}
