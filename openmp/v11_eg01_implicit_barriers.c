#include <omp.h>
#include <stdio.h>

#define N 4

int big_calc1(int id) {
  return id * 10;
}

int big_calc2(int i, int* array) {
  return i;
}

int big_calc3(int i, int* array) {
  return i;
}

int big_calc4(int id) {
  return id * 2;
}

/**
 * Entry point.
 */
int main(void) {

  int A[4] = { 1, 1, 1, 1 };
  int B[4] = { 2, 2, 2, 2 };
  int C[4] = { 3, 3, 3, 3 };
  int i, id;

  #pragma omp parallel shared(A, B, C) private (id)
  {
    id = omp_get_thread_num();
    printf("[%d] Hello!\n", id);

    A[id] = big_calc1(id);

    #pragma omp barrier

    // There is an implicit barrier at the end of every worksharing construct,
    // such as loops.
    #pragma omp for
    for (i = 0; i < N; ++i) {
      C[i] = big_calc2(i, A);
    }

    // There is NOT going to be an implicit barrier at the end of this loop.
    #pragma omp for nowait
    for (i = 0; i < N; ++i) {
      B[i] = big_calc3(i, C);
    }

    A[id] = big_calc4(id);
  }

  printf("Done!\n");
  return 0;
}

