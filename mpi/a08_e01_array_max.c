/**
 * Finds the maximum value from an array of integers.
 * Rank 0 is responsible for initializing the array.
 *
 * Compile and run:
 * mpicc -o a08_e01_array_max a08_e01_array_max.c
 * mpiexec -n 4 ./a08_e01_array_max
 */

#include <assert.h>
#include <limits.h>   /* INT_MIN */
#include <mpi.h>
#include <stdio.h>    /* printf */
#include <stdlib.h>   /* malloc, free */
#include <time.h>

/**
 * Calculates the maximum value from an array.
 * @param  ary  The array.
 * @param  size The array's size.
 * @return      The maximum value.
 */
int array_max(int* ary, int size) {
  int max = INT_MIN;
  for (int i = 0; i < size; ++i) {
    if (ary[i] > max) max = ary[i];
  }
  return max;
}

/**
 * Initializes an array of size `ary_size`.
 * @param  ary_size Size of the array
 * @return          Pointer to the array
 */
int* init_array(int ary_size) {
  int* ary = (int*)malloc(sizeof(int) * ary_size);
  for (int i = 0; i < ary_size; ++i) {
    ary[i] = (int)((rand() / (float)RAND_MAX) * 100);
  }
  return ary;
}

/**
 * Calculates the necessary parameters for MPI_Scatterv.
 * @param  ary_size      Size of the entire array of data
 * @param  world_size    Number of processes
 * @param  sendcounts    Array for the number of elements for each process.
 * @param  displacements Array for the displacement index for each process.
 */
void calculate_for_scatterv(int ary_size, int world_size, int* sendcounts,
                           int* displacements) {

  // "Default" number of elements each process will receive.
  int elements_per_process = ary_size / world_size;

  // How many elements will have to be redistributed.
  int remainder = ary_size % world_size;

  int i, count, displacement = 0;

  for (i = 0; i < world_size; ++i) {
    if (i < world_size - remainder) {
      count = elements_per_process;
    }
    else {
      count = elements_per_process + 1;
    }

    sendcounts[i] = count;
    displacements[i] = displacement;
    displacement += count;
  }
}

int main(int argc, char** argv) {

  // Initialize MPI.
  MPI_Init(NULL, NULL);

  // Seed random number generator.
  srand(time(NULL));

  const int ARRAY_SIZE = 10;

  // Get MPI information.
  int my_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  printf("[%d] Hello!\n", my_rank);

   // Array specifying the length of each split.
  int* sendcounts = (int*)malloc(sizeof(int) * world_size);
  assert(sendcounts != NULL);

  // Array specifying the index at which each split begins.
  int* displacements = (int*)malloc(sizeof(int) * world_size);
  assert(displacements != NULL);

  // Split the array into (world_size) groups.
  calculate_for_scatterv(ARRAY_SIZE, world_size, sendcounts, displacements);

  int i;
  int* array;

  if(my_rank == 0) {

    // Initialize array.
    array = init_array(ARRAY_SIZE);

    printf("Calculated displacements:\n");
    printf("sendcounts:    ");
    for (i = 0; i < world_size; ++i) printf("    %d", sendcounts[i]);
    printf("\ndisplacements: ");
    for (i = 0; i < world_size; ++i) printf("    %d", displacements[i]);
    printf("\n");
  }

  // Allocate one buffer for each process that will receive data.
  int* splitbuf = (int*)malloc(sizeof(int) * ((ARRAY_SIZE / world_size) + 1));

  // Receive data.
  MPI_Scatterv(array, sendcounts, displacements, MPI_INT, splitbuf,
               (ARRAY_SIZE / world_size) + 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Each process prints what they received from root.
  printf("[%d] ", my_rank);
  for (i = 0; i < sendcounts[my_rank]; ++i) printf("%d ", splitbuf[i]);
  printf("\n");

  // Each process calculates the maximum for its array.
  int splitmax = array_max(splitbuf, sendcounts[my_rank]);
  printf("[%d] max: %d\n", my_rank, splitmax);

  // Reduce results in root process.
  int global_max;
  MPI_Reduce(&splitmax, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    printf("[%d] Global maximum: %d\n", my_rank, global_max);
  }

  // Clean up.
  free(array);
  free(splitbuf);
  free(sendcounts);
  free(displacements);

  MPI_Finalize();
  return 0;
}
