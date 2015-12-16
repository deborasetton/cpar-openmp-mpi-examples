/**
 * Parallel calculation of pi using omp reduction.
 */

#include <stdio.h>

static long num_steps = 99000000;
double step;

int main(void) {
  int i;
  double x, pi, sum = 0.0;

  printf("num_steps = %ld\n", num_steps);

  step = 1.0 / (double) num_steps;

  #pragma omp parallel for private(x) reduction (+:sum)
  for (i = 0; i < num_steps; ++i) {
    x = (i + 0.5) * step;
    sum += 4.0 / (1.0 + x * x);
  }

  pi = step * sum;
  printf("Pi is: %f\n", pi);
  return 0;
}
