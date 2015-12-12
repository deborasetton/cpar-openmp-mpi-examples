/**
 * Finds the first occurrence of a given number in multiple arrays, one for
 * each rank in MPI_COMM_WORLD, using a distributed search with MPI_Bsend and
 * MPI_Gather.
 *
 * Compile & run:
 * $ mpicc -o a10_e01_find_element a10_e01_find_element.c
 * $ mpiexec -n 10 ./a10_e01_find_element
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// All ranks will create an array of random integers of this size.
const int ARRAY_SIZE = 20;

void print_array(int* array, int size);

/**
 * Entry point.
 */
int main(int argc, char** argv) {

  // Initialize MPI.
  MPI_Init(NULL, NULL);

  int world_size, my_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // All ranks: initialize local array with random values and print the array.

  // Make sure different ranks get different random values.
  srand(time(NULL) + my_rank);

  int i;
  int local_array[ARRAY_SIZE];
  for (i = 0; i < ARRAY_SIZE; ++i) {
    local_array[i] = (rand() / (float)RAND_MAX) * 100;
  }
  print_array(local_array, ARRAY_SIZE);

  // Value to search for.
  int target;

  // Master rank: ask for the number to search for.
  if (my_rank == 0) {
    printf("Type the number you want to look for:\n");
    scanf("%d", &target);
  }

  // Broadcast target value to other ranks.
  MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // All ranks: search for target.
  int occurrence_idx = -1;
  for (i = 0; i < ARRAY_SIZE; ++i) {
    if (local_array[i] == target) {
      occurrence_idx = i;
      break;
    }
  }

  printf("[%d] my idx: %d\n", my_rank, occurrence_idx);

  int occurrences[world_size];
  MPI_Gather(&occurrence_idx, 1, MPI_INT, occurrences, 1, MPI_INT, 0,
             MPI_COMM_WORLD);

  if (my_rank == 0) {
    printf("\nSearch completed. Results:\n");
    for (i = 0; i < world_size; ++i) {
      printf("[%d] Index: %d\n", i, occurrences[i]);
    }
  }

  // Clean up.
  MPI_Finalize();
  return 0;
}

/**
 * Pretty-print an array.
 */
void print_array(int* array, int size) {

  /** Uncomment to print the array indices as well */
  // printf("[");
  // for (int i = 0; i < size; ++i) {
  //   printf("%4d ", i);
  // }
  // printf("]\n");

  printf("[");
  for (int i = 0; i < size; ++i) {
    printf("%4d ", array[i]);
  }
  printf("]\n");
}
