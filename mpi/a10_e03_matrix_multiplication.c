/**
 * Calculates W = (X + Y + Z) * T, where:
 *   - X = A * B
 *   - Y = C * D
 *   - Z = E * F
 *   - Each of these is a square matrix of floats.
 *
 * Task 1: initialize A and B and sends each calculated row of X to root using
 *        MPI_Isend and MPI_Wait.
 * Task 2: initialize C and D and sends each calculated row of Y to root using
 *        MPI_Isend and MPI_Wait.
 * Task 3: initialize E and F and sends each calculated row of Z to root using
 *        MPI_Isend and MPI_Wait.
 * Task 4: initialize T and receives lines of X, Y and Z from other ranks using
 *        MPI_Irecv and MPI_Wait to calculate W.
 *
 * Recommended way of running locally, to see each process output in its own
 * terminal:
 * # Compile
 * mpicc -o a10_e03_matrix_multiplication a10_e03_matrix_multiplication.c
 * # Setup
 * touch a10_e03.1.0 a10_e03.1.1 a10_e03.1.2 a10_e03.1.3
 *
 * # Setup tmux. Open 4 terminals and, on each one:
 * tail -f a10_e03.1.0
 * tail -f a10_e03.1.1
 * tail -f a10_e03.1.2
 * tail -f a10_e03.1.3
 *
 * # On a different terminal:
 * mpiexec --output-filename matrix_sum_out -n 4 ./a10_e03_matrix_multiplication
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Matrix dimensions.
const int N = 2;

// Prototypes.
void print_matrix(void* m, int rows, int cols);
void print_array(float* array, int size);

/**
 * [task description]
 */
void task_ABX(int my_rank) {
  float mA[N][N];
  float mB[N][N];
  float mX[N][N];
  int i, j, k;
  MPI_Request requests[N];

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      mA[i][j] = (rand() / (float)RAND_MAX) * 100;
      mB[i][j] = (rand() / (float)RAND_MAX) * 100;
    }
  }

  printf("Matrix A:\n");
  print_matrix(mA, N, N);

  printf("\nMatrix B:\n");
  print_matrix(mB, N, N);
  printf("\n");

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {

      mX[i][j] = 0;
      for (int k = 0; k < N; ++k) {
        mX[i][j] += mA[i][k] * mB[k][j];
      }
    }

    printf("[%d] Row %d of mX:", my_rank, i);
    print_array(mX[i], N);

    MPI_Isend(mX[i], N, MPI_FLOAT, 0, i, MPI_COMM_WORLD, &requests[i]);
  }

  for (i = 0; i < N; ++i) {
    printf("[%d] Waiting for row %d\n", my_rank, i);
    MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
  }
}

void task_master(int my_rank) {
  int i, j, k;

  float mT[N][N];
  float mW[N][N];
  float rowX[N];
  float rowY[N];
  float rowZ[N];
  float row[N];
  MPI_Request requestX;
  MPI_Request requestY;
  MPI_Request requestZ;

  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      mT[i][j] = 0.0;
      if (i == j) mT[i][j] = 1.0;
    }
  }

  printf("\nMatrix T:\n");
  print_matrix(mT, N, N);
  printf("\n");

  for (i = 0; i < N; ++i) {
    MPI_Irecv(rowX, N, MPI_FLOAT, 1, i, MPI_COMM_WORLD, &requestX);
    MPI_Irecv(rowY, N, MPI_FLOAT, 2, i, MPI_COMM_WORLD, &requestY);
    MPI_Irecv(rowZ, N, MPI_FLOAT, 3, i, MPI_COMM_WORLD, &requestZ);
    MPI_Wait(&requestX, MPI_STATUS_IGNORE);
    MPI_Wait(&requestY, MPI_STATUS_IGNORE);
    MPI_Wait(&requestZ, MPI_STATUS_IGNORE);

     // W = (X + Y + Z) * T

    printf("[%d] Received row %d of mX: ", my_rank, i);
    print_array(rowX, N);

    printf("[%d] Received row %d of mY: ", my_rank, i);
    print_array(rowY, N);

    printf("[%d] Received row %d of mZ: ", my_rank, i);
    print_array(rowZ, N);

    // Sum rows X, Y and Z.
    for (j = 0; j < N; ++j) {
      row[j] = rowX[j] + rowY[j] + rowZ[j];
      mW[i][j] = 0;
    }

    // Calculate a row of mW.
    for (j = 0; j < N; ++j) {
      for (k = 0; k < N; ++k) {
        mW[i][j] += row[j] * mT[k][j];
      }
    }
  }

  printf("\nMatrix W:\n");
  print_matrix(mW, N, N);
}

/**
 * Entry point.
 */
int main(int argc, char** argv) {

  // Initialize MPI.
  MPI_Init(NULL, NULL);

  int world_size, my_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (world_size != 4) {
    fprintf(stderr, "This program needs exactly 4 processes to run.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Make sure different ranks get different random values.
  srand(time(NULL) + my_rank);

  if (my_rank == 0) {
    task_master(my_rank);
  }
  else {
    task_ABX(my_rank);
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
  float (*matrix)[cols] = m;

  // Print row of column indexes.
  for (int i = 0; i < cols; ++i) {
    if (i == 0) { printf("   "); }
    printf("%9d ", i);
  }
  printf("\n");

  // Print row of dashes ('-').
  for (int i = 0; i < cols; ++i) {
    if (i == 0) { printf("   "); }
    printf("----------");
  }
  printf("\n");

  // Print matrix.
  for(int i = 0; i < rows; ++i) {
    printf("%d |", i);
    for (int j = 0; j < cols; ++j) {
      // Alternative for matrix[i][j]: *(*(matrix + i) + j)
      printf("%f ", *(*(matrix + i) + j));
    }
    printf("\n");
  }
}

/**
 * Pretty-print an array.
 */
void print_array(float* array, int size) {
  printf("[");
  for (int i = 0; i < size; ++i) {
    printf("%f ", array[i]);
  }
  printf("]\n");
}
