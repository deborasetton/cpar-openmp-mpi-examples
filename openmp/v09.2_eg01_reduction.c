#include <omp.h>
#include <stdio.h>

#define MAX 10

int main(void) {

  // Initialize avg with the identity value for '+'.
  double avg = 0.0;
  double A[MAX] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  int i;

  #pragma omp parallel for reduction (+:avg)
  for (i = 0; i < MAX; ++i) {
    avg += A[i];
  }

  avg = avg / MAX;
  printf("Avg = %lf", avg);

  return 0;
}
