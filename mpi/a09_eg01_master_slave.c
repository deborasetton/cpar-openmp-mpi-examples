/**
 * Example of master-slave program with MPI.
 */

#include <mpi.h>
#include <stdio.h>

#define WORKTAG        1
#define DIETAG         2
#define NUM_WORK_REQS  5

void master();
void slave();

// Defines an array of 10 elements, each being a pointer to a function
// that is void and receives has no arguments.
void (*tab_func[5])();

void func0() {
  int result = 0;
  MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void func1() {
  int result = 1;
  MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void func2() {
  int result = 2;
  MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void func3() {
  int result = 3;
  MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void func4() {
  int result = 4;
  MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

/**
 * Entry point.
 */
int main(int argc, char** argv) {

  // Initialize function table.
  tab_func[0] = func0;
  tab_func[1] = func1;
  tab_func[2] = func2;
  tab_func[3] = func3;
  tab_func[4] = func4;

  // Initialize MPI.
  MPI_Init(&argc, &argv);

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0) {
    master();
  }
  else {
    slave();
  }

  MPI_Finalize();
  return 0;
}

/**
 * Master function.
 * Assigns work to slaves and collects results.
 */
void master() {
  int rank, work, result;
  MPI_Status status;

  // Get number of processes.
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  work = NUM_WORK_REQS - 1;

  // Assigns a first round of work to each slave process.
  for (rank = 1; rank < world_size; ++rank) {
    MPI_Send(&work, 1, MPI_INT, rank, WORKTAG, MPI_COMM_WORLD);
    work--;
  }

  // While there's work to be assigned, receive a result and assign more work.
  while (work >= 0) {
    MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    MPI_Send(&work, 1, MPI_INT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
    work--;
  }

  // No more work to be assigned, so just receive and tell processes to stop.
  for (rank = 1; rank < world_size; rank++) {
    MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    // Tell slaves to quit.
    MPI_Send(0, 0, MPI_INT, status.MPI_SOURCE, DIETAG, MPI_COMM_WORLD);
  }
}

/**
 * Slave function.
 */
void slave() {
  int result, work, my_rank;
  MPI_Status status;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  printf("[%d] Starting slave process\n", my_rank);

  for(;;) {
    MPI_Recv(&work, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == DIETAG) {
      printf("[%d] Breaking\n", my_rank);
      break;
    }

    printf("[%d] Working on: %d\n", my_rank, work);
    (*tab_func[work])();
  }
}
