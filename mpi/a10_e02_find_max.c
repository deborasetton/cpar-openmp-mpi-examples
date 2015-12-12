/**
 * Finds the max value in a matrix of size ROWS x COLS, initialized with
 * random values, using a distributed search, per row, implemented with
 * MPI_Scatter and MPI_Reduce.
 *
 * Compile & run:
 * $ mpicc -o a10_e02_find_max a10_e02_find_max.c
 * $ mpiexec -n 5 ./a10_e02_find_max
 */

#include <limits.h>   /* INT_MIN */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Matrix dimensions.
const int ROWS = 10;
const int COLS = 15;

// Prototypes.
void print_matrix(void* m, int rows, int cols);
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

  // Since we don't want to use MPI_Scatterv, the number of rows must be
  // divisible by the number of processes.
  if (ROWS % world_size != 0) {
    fprintf(stderr,
            "Use a number of processes that is multiple of %d (used: %d)\n",
            ROWS, world_size);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Seed random number generator.
  srand(time(NULL));

  // How many rows from the original matrix each process will receive.
  int rows_per_process = ROWS / world_size;

  int (*matrix)[COLS];     // The whole matrix.
  int (*submatrix)[COLS];  // The submatrix, per process.
  submatrix = malloc(sizeof(int) * rows_per_process * COLS);

  // Initialize matrix.
  if (my_rank == 0) {
    matrix = malloc(sizeof(int) * ROWS * COLS);
    int i, j;
    for (i = 0; i < ROWS; ++i) {
      for (j = 0; j < COLS; ++j) {
        matrix[i][j] = (rand() / (float)RAND_MAX) * 100;
      }
    }
    print_matrix(matrix, ROWS, COLS);
    printf("\n");
  }

  // Assign a group of rows to each rank.
  MPI_Scatter(matrix, rows_per_process * COLS, MPI_INT, submatrix,
              rows_per_process * COLS, MPI_INT, 0, MPI_COMM_WORLD);

  printf("[%d] My rows:\n", my_rank);
  int i;
  for (i = 0; i < rows_per_process; ++i) {
    printf("[%d] ", my_rank);
    print_array(submatrix[i], COLS);
  }

  // All processes: find their maximum.
  int max = INT_MIN, j;
  for (i = 0; i < rows_per_process; ++i) {
    for (j = 0; j < COLS; ++j) {
      if (submatrix[i][j] > max) {
        max = submatrix[i][j];
      }
    }
  }

  printf("[%d] My max: %d\n", my_rank, max);

  int global_max;
  MPI_Reduce(&max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    printf("Done! Global max is: %d\n", global_max);
    free(matrix);
  }

  // Clean up.
  MPI_Finalize();
  return 0;
}

/**
 * Pretty-prints a matrix.
 * @param m    The matrix
 * @param rows Number of rows
 * @param cols Number of columns
 */
void print_matrix(void* m, int rows, int cols) {
  // `matrix` is a pointer to an array of cols integers.
  int (*matrix)[cols] = m;

  // Print row of column indexes.
  for (int i = 0; i < cols; ++i) {
    if (i == 0) { printf("      "); }
    printf("%4d ", i);
  }
  printf("\n");

  // Print row of dashes ('-').
  for (int i = 0; i < cols; ++i) {
    if (i == 0) { printf("     "); }
    printf("-----");
  }
  printf("\n");

  // Print matrix.
  for(int i = 0; i < rows; ++i) {
    printf("%4d |", i);
    for (int j = 0; j < cols; ++j) {
      // Alternative for matrix[i][j]: *(*(matrix + i) + j)
      printf("%4d ", *(*(matrix + i) + j));
    }
    printf("\n");
  }
}

/**
 * Pretty-print an array.
 */
void print_array(int* array, int size) {
  printf("[");
  for (int i = 0; i < size; ++i) {
    printf("%4d ", array[i]);
  }
  printf("]\n");
}
