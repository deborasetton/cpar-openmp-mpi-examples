#include <omp.h>
#include <stdio.h>

const int N = 10;

int num;

void func(void) {
  printf("[%d] func, num = %d\n", omp_get_thread_num(), num);
}

int main(void) {
  int i;

  num = 99;

  #pragma omp parallel for firstprivate(num)
  for (i = 0; i < N; ++i) {
    printf("[%d] i = %d, num = %d\n", omp_get_thread_num(), i, num);
    func();
    num += i;
  }
  return 0;
}
