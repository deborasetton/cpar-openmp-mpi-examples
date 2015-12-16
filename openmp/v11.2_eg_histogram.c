/**
 * Example of using locks in OpenMP (omp_init_lock, omp_set_lock,
 * omp_unset_lock, omp_destroy_lock) to calculate a histogram.
 *
 * Note: the parallel version of this program will only have better performance
 * than the sequential version if the probability of collision of buckets is
 * low, which means NBUCKETS is very large. In this case, we'll have mostly
 * uncontended locks.
 */

#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>  /* malloc, rand, srand */
#include <time.h>    /* time */

#define NBUCKETS   100000
#define NVALS      10000000

// Set to 1 to print the sample value and the final histogram.
#define DEBUG      0

void generate_samples(int* samples);

/**
 * Entry point
 */
int main(void) {

  omp_lock_t hist_locks[NBUCKETS];
  int hist[NBUCKETS];
  int* samples;
  int i, ival;

  samples = (int*)malloc(sizeof(int) * NVALS);
  assert(samples != NULL);

  generate_samples(samples);

  #pragma omp parallel
  {
    printf("[%d] Total threads: %d\n", omp_get_thread_num(),
           omp_get_num_threads());

    #pragma omp for
    for (i = 0; i < NBUCKETS; ++i) {
      omp_init_lock(&hist_locks[i]);
      hist[i] = 0;
    }
  }

  #pragma omp parallel for private(ival)
  for (i = 0; i < NVALS; ++i) {
    ival = samples[i];
    omp_set_lock(&hist_locks[ival]);
    hist[ival]++;
    omp_unset_lock(&hist_locks[ival]);
  }

  #pragma omp parallel for
  for (i = 0; i < NBUCKETS; ++i) {
    omp_destroy_lock(&hist_locks[i]);
  }

  if (DEBUG) {
    printf("Histogram:\n");
    for (i = 0; i < NBUCKETS; ++i) {
      printf("%d ---> %d\n", i, hist[i]);
    }
  }

}

/**
 * Populate the array with random values between 0 and NBUCKETS.
 */
void generate_samples(int* samples) {
  srand(time(NULL));

  for (int i = 0; i < NVALS; ++i) {
    samples[i] = rand() % NBUCKETS;
  }

  if (DEBUG) {
    printf("Sample values:\n");
    for (int i = 0; i < NVALS; ++i) {
      printf("%d", samples[i]);
      if (i != NVALS - 1) { printf(", "); }
    }
    printf("\n\n");
  }
}
