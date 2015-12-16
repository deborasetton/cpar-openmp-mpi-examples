/*
 * Simple atomic example.
 * https://youtu.be/WcPZLJKtywc?list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG&t=438
 */

#include <stdio.h>
#include <omp.h>

int main(void) {

  int sum = 0;

  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    printf("Hey, I'm thread #%d\n", id);

    #pragma omp atomic
    // Has to be a really simple expression:
    // "The statement for 'atomic' must be an expression statement of form
    // '++x;', '--x;', 'x++;', 'x--;', 'x binop= expr;', 'x = x binop expr'
    // or 'x = expr binop x', where x is an l-value expression with scalar
    // type".
    sum += 1;
  }

  printf("Final value of sum is: %d.\n", sum);

  printf("Done!\n");
  return 0;
}
