/*
 * Barrier example.
 * https://youtu.be/WcPZLJKtywc?list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG&t=179
 */

#include <stdio.h>
#include <omp.h>

int main(void) {

  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    printf("Hey, I'm thread #%d\n", id);

    #pragma omp barrier
    printf("Thread #%d after barrier!\n", id);
  }

  printf("Done!\n");
  return 0;
}
