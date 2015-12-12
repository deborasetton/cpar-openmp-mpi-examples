/**
 * Example of sum of matrices using MPI_Bsend.
 * There are 3 slave processes and 1 master process:
 * - TASK1_RANK: initializes matrix mX and perform computations on the rows of
 *   mX. For each computed row, uses MPI_Bsend to send the row to the master
 *   process.
 * - TASK2_RANK: same as TASK1_RANK, but for matrix mY.
 * - TASK2_RANK: same as TASK1_RANK, but for matrix mZ.
 * - MASTER_RANK: calculates mX + mY + mZ row by row and prints the result.
 *
 * Recommended way of running, to see each process output in its own terminal:
 * # Compile
 * mpicc -o a09_e02_matrix_sum a09_e02_matrix_sum.c
 * # Setup
 * touch matrix_sum_out.1.0 matrix_sum_out.1.1 matrix_sum_out.1.2 matrix_sum_out.1.3
 *
 * # Setup tmux. Open 4 terminals and, on each one:
 * tail -f matrix_sum_out.1.0
 * tail -f matrix_sum_out.1.1
 * tail -f matrix_sum_out.1.2
 * tail -f matrix_sum_out.1.3
 *
 * # On a different terminal:
 * mpiexec --output-filename matrix_sum_out -n 4 ./a09_e02_matrix_sum
 *
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TASK1_RANK  1  // This rank will execute task1()
#define TASK2_RANK  2  // This rank will execute task2()
#define TASK3_RANK  3  // This rank will execute task3()
#define MASTER_RANK 0  // This rank will execute task4()

/* Prototypes */
void print_matrix(void* m, int rows, int cols);
void task1(int my_rank);
void task2(int my_rank);
void task3(int my_rank);
void task4(int my_rank);

/* Constants */
const int ROWS = 3;
const int COLS = 3;

/**
 * Entry point.
 */
int main(int argc, char** argv) {

  // Initialize MPI.
  MPI_Init(NULL, NULL);

  int my_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_size != 4) {
    fprintf(stderr, "This program needs exactly 4 processes to run.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Everybody create their buffers for Bsend!

  // Calculate size of 1 message
  int bufsize;
  MPI_Pack_size(COLS, MPI_INT, MPI_COMM_WORLD, &bufsize);

  // The necessary amount for each message is multiplied by the number of
  // messages that will be sent to get the final necessary amount.
  bufsize = ROWS * (bufsize + MPI_BSEND_OVERHEAD);

  if (my_rank == MASTER_RANK) {
    // For each row of data sent by other processes, the master process will
    // receive 3, so allocate 3x the amount.
    bufsize = bufsize * 3;
  }

  // Allocate the user buffer and hand it over to MPI.
  int* sendbuf = (int*)malloc(sizeof(int) * bufsize);
  MPI_Buffer_attach(sendbuf, bufsize);

  switch (my_rank) {
    case MASTER_RANK:
      task4(my_rank); break;
    case 1:
      task1(my_rank); break;
    case 2:
      task2(my_rank); break;
    case 3:
      task3(my_rank); break;
  }

  // Clean up.
  MPI_Buffer_detach(sendbuf, &bufsize);
  free(sendbuf);

  MPI_Finalize();
  return 0;
}

/**
 * Initializes matrix X and does some computations with it.
 */
void task1(int my_rank) {
  printf("[%d] Task 1\n", my_rank);

  // Initialization of matrix mX.
  int mX[ROWS][COLS], i, j;
  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mX[i][j] = 1;
    }
  }
  print_matrix(&mX, ROWS, COLS);

  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mX[i][j] = mX[i][j] * 10;
    }
    MPI_Bsend(mX[i], COLS, MPI_INT, MASTER_RANK, i, MPI_COMM_WORLD);
  }
  print_matrix(&mX, ROWS, COLS);
}

/**
 * Initializes matrix Y and does some computations with it.
 */
void task2(int my_rank) {
  printf("[%d] Task 2\n", my_rank);

  // Initialization of matrix mY.
  int mY[ROWS][COLS], i, j;
  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mY[i][j] = 2;
    }
  }
  print_matrix(&mY, ROWS, COLS);

  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mY[i][j] = mY[i][j] + 3;
    }
    MPI_Bsend(mY[i], COLS, MPI_INT, MASTER_RANK, i, MPI_COMM_WORLD);
  }
  print_matrix(&mY, ROWS, COLS);
}

/**
 * Initializes matrix Z and does some computations with it.
 */
void task3(int my_rank) {
  printf("[%d] Task 3\n", my_rank);

  // Initialization of matrix mZ.
  int mZ[ROWS][COLS], i, j;
  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mZ[i][j] = 3;
    }
  }
  print_matrix(&mZ, ROWS, COLS);

  for (i = 0; i < ROWS; ++i) {
    for (j = 0; j < COLS; ++j) {
      mZ[i][j] = mZ[i][j] + 3 * i;
    }
    MPI_Bsend(mZ[i], COLS, MPI_INT, MASTER_RANK, i, MPI_COMM_WORLD);
  }
  print_matrix(&mZ, ROWS, COLS);
}

/**
 * Aggregates results from tasks 1, 2 and 3.
 */
void task4(int my_rank) {
  printf("[%d] Task 4\n", my_rank);

  int rowX[COLS];
  int rowY[COLS];
  int rowZ[COLS];
  int result[ROWS][COLS];

  int i, j;
  for (i = 0; i < ROWS; ++i) {
    printf("[%d] Receiving data for row %d\n", my_rank, i);

    MPI_Recv(rowX, COLS, MPI_INT, TASK1_RANK, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%d] Received row %d from rank %d\n", my_rank, i, TASK1_RANK);

    MPI_Recv(rowY, COLS, MPI_INT, TASK2_RANK, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%d] Received row %d from rank %d\n", my_rank, i, TASK2_RANK);

    MPI_Recv(rowZ, COLS, MPI_INT, TASK3_RANK, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%d] Received row %d from rank %d\n", my_rank, i, TASK3_RANK);

    for (j = 0; j < COLS; ++j) {
      result[i][j] = rowX[j] + rowY[j] + rowZ[j];
    }
  }

  print_matrix(&result, ROWS, COLS);
}

/**
 * Pretty-prints a matrix.
 * @param m    The matrix
 * @param rows Number of rows
 * @param cols Number of columns
 */
void print_matrix(void* m, int rows, int cols) {
  printf("Ok!\n");

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
