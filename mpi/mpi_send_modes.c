/**
 * Illustration of the different send modes in MPI.
 * For a summary of each one, see:
 * http://www.mcs.anl.gov/research/projects/mpi/sendmode.html
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>   /* malloc */
#include <unistd.h>   /* sleep */

void (*examples[8])(int my_rank);

void ex_send();
void ex_bsend();
void ex_ssend();
void ex_rsend();
void ex_isend();
void ex_ibsend();
void ex_issend();
void ex_irsend();

int choose_from_menu();

/**
 * Entry point.
 */
int main(int argc, char** argv) {

  // Initialize function table.
  examples[0] = ex_send;
  examples[1] = ex_bsend;
  examples[2] = ex_ssend;
  examples[3] = ex_rsend;
  examples[4] = ex_isend;
  examples[5] = ex_ibsend;
  examples[6] = ex_issend;
  examples[7] = ex_irsend;

  MPI_Init(NULL, NULL);

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_size != 2) {
    fprintf(stderr, "World size must be two for %s\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int example_num;

  if (my_rank == 0) {
    example_num = choose_from_menu();
  }

  // This is synchronous.
  MPI_Bcast(&example_num, 1, MPI_INT, 0, MPI_COMM_WORLD);

  printf("[%d] Running example %d \n", my_rank, example_num);
  (*examples[example_num - 1])(my_rank);

  MPI_Finalize();
  return 0;
}

int choose_from_menu() {
  printf("Choose the example to execute:\n");
  printf("1. MPI_Send   (standard mode)\n");
  printf("2. MPI_Bsend  (buffered)\n");
  printf("3. MPI_Ssend  (synchronous)\n");
  printf("4. MPI_Rsend  (ready)\n");
  printf("5. MPI_Isend  (immediate standard)\n");
  printf("6. MPI_Ibsend (immediate buffered)\n");
  printf("7. MPI_Issend (immediate synchronous)\n");
  printf("8. MPI_Irsend (immediate ready)\n");
  printf("Type a number:\n");

  int option;
  scanf("%d", &option);

  if (option < 1 || option > 8) {
    printf("%d is invalid.\n\n", option);
    return choose_from_menu();
  }

  printf("\n");
  return option;
}

/**
 * Example of MPI_Send usage.
 * @param my_rank Rank number of the process that will execute the function.
 */
void ex_send(int my_rank) {
  printf("[%d] Running example: MPI_Send\n", my_rank);

  // The MPI_Send call will be synchronous or asynchronous depending on the
  // size of the message. If you change this value to 100000, you'll see that
  // the time to MPI_Send approaches the sleep time of rank 1.
  const int ARRAY_SIZE = 10;

  int* array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
  assert(array != NULL);

  if (my_rank == 0) {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
      array[i] = i; // Populate array with something.
    }

    double time_to_send = 0.0;
    time_to_send -= MPI_Wtime();
    MPI_Send(array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    time_to_send += MPI_Wtime();
    printf("[%d] Time to MPI_Send: %lf\n", my_rank, time_to_send);
  }
  else {
    sleep(2);
    MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[%d] Last element of array is: %d\n", my_rank,
           array[ARRAY_SIZE -1 ]);
  }

  // Clean up.
  free(array);
}

/**
 * Example of MPI_Bsend usage.
 * @param my_rank Rank number of the process that will execute the function.
 */
void ex_bsend(int my_rank) {
  printf("[%d] Running example: MPI_Bsend\n", my_rank);

  const int ARRAY_SIZE = 100000;

  // Data that will be exchanged between rank 0 and rank 1.
  int* data_array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
  assert(data_array != NULL);

  if (my_rank == 0) {
    // Rank 0 will send the same array twice, but in buffered fashion, so
    // we need to allocate a buffer big enough for two arrays of size ARRAY_SIZE
    // plus the overhead space required by MPI.

    int bufsize;
    MPI_Pack_size(ARRAY_SIZE, MPI_INT, MPI_COMM_WORLD, &bufsize);

    // The necessary amount for each message is multiplied by the number of
    // messages that will be sent to get the final necessary amount.
    bufsize = 2 * (bufsize + MPI_BSEND_OVERHEAD);

    // Allocate the user buffer and hand it over to MPI.
    int* sendbuf = (int*)malloc(sizeof(int) * bufsize);
    MPI_Buffer_attach(sendbuf, bufsize);

    for (int i = 0; i < ARRAY_SIZE; ++i) {
      data_array[i] = i; // Populate array with something.
    }

    // 1st Bsend call.
    double time_to_send = 0.0;
    time_to_send -= MPI_Wtime();
    MPI_Bsend(data_array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    time_to_send += MPI_Wtime();
    printf("[%d] Time to 1st MPI_Send: %lf\n", my_rank, time_to_send);

    // 2nd Bsend call.
    time_to_send = 0.0;
    time_to_send -= MPI_Wtime();
    // Modify the last element of the array so we can see the change in
    // the output.
    data_array[ARRAY_SIZE - 1] = 123456;
    MPI_Bsend(data_array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    time_to_send += MPI_Wtime();
    printf("[%d] Time to 2nd MPI_Send: %lf\n", my_rank, time_to_send);

    MPI_Buffer_detach(sendbuf, &bufsize);
    free(sendbuf);
  }
  else {
    // Rank 1 receives two arrays of the same size and prints the last element
    // of each one.

    sleep(2);
    MPI_Recv(data_array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[%d] Last element of 1st array is: %d\n", my_rank,
           data_array[ARRAY_SIZE -1 ]);

    // Receive again.

    sleep(2);
    MPI_Recv(data_array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[%d] Last element of 2nd array is: %d\n", my_rank,
           data_array[ARRAY_SIZE -1 ]);
  }

  // Clean up.
  free(data_array);
}

/**
 * Example of MPI_Ssend usage.
 * Ssend is always synchronous, i.e., will always wait for the matching receive.
 * It's like we're always exceeding the limit if we were using standard
 * MPI_Send.
 */
void ex_ssend(int my_rank) {
  printf("[%d] Running example: MPI_Ssend\n", my_rank);

  const int ARRAY_SIZE = 10;

  int* array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
  assert(array != NULL);

  if (my_rank == 0) {
    for (int i = 0; i < ARRAY_SIZE; ++i) {
      array[i] = i; // Populate array with something.
    }

    // Since Ssend is *always* synchronous, the first time to send will be
    // almost equal to the first sleep call of rank 1, and likewise for the
    // second ssend.

    double time_to_send = 0.0;
    time_to_send -= MPI_Wtime();
    MPI_Ssend(array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    time_to_send += MPI_Wtime();
    printf("[%d] Time to 1st MPI_Ssend: %lf\n", my_rank, time_to_send);

    time_to_send = 0.0;
    time_to_send -= MPI_Wtime();
    MPI_Ssend(array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    time_to_send += MPI_Wtime();
    printf("[%d] Time to 2nd MPI_Ssend: %lf\n", my_rank, time_to_send);
  }
  else {
    sleep(2);
    MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[%d] Last element of 1st array is: %d\n", my_rank,
           array[ARRAY_SIZE -1 ]);

    sleep(4);
    MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    printf("[%d] Last element of 2nd array is: %d\n", my_rank,
           array[ARRAY_SIZE -1 ]);
  }

  // Clean up.
  free(array);
}

/**
 * Example of MPI_Rsend usage.
 * MPI_Rsend requires that a previous matching receive call has already
 * happened by the time MPI_Rsend is called. It's the programmer's
 * responsibility to guarantee this correct ordering of events.
 *
 * ***********************************************************************
 * Note: this is an example of calling MPI_Rsend the _wrong_ way, since
 * the MPI_Rsend call happens before the MPI_Recv call. However, since the
 * behavior for this error is undefined by the standard, the code actually
 * works as if MPI_Rsend was a regular MPI_Send.
 *
 * More info:
 * http://stackoverflow.com/questions/34223226/mpi-rsend-not-raising-error-when-it-should
 * ***********************************************************************
 */
void ex_rsend(int my_rank) {
  printf("[%d] Running example: MPI_Rsend\n", my_rank);

  const int ARRAY_SIZE = 100000;

  int* array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
  assert(array != NULL);

  if (my_rank == 0) {

    for (int i = 0; i < ARRAY_SIZE; ++i) {
      array[i] = i; // Populate array with something.
    }

    int error_val;
    printf("[%d] Called Rsend\n", my_rank);
    error_val = MPI_Rsend(array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    printf("[%d] Done with status %d\n", my_rank, error_val);
  }
  else {
    sleep(5);
    MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%d] Last element of array is: %d\n", my_rank,
           array[ARRAY_SIZE -1 ]);
  }

}

/**
 * Example of MPI_Isend usage.
 * MPI_Isend is the *immediate* version of MPI_Send, which means it doesn't
 * block (execution continues immediately), and the user has to use the
 * MPI_Request object to poll for the operation status.
 */
void ex_isend(int my_rank) {
  printf("[%d] Running example: MPI_Isend\n", my_rank);

  const int ARRAY_SIZE = 100000;
  int* array = (int*)malloc(sizeof(int) * ARRAY_SIZE);
  assert(array != NULL);

  if (my_rank == 0) {

    for (int i = 0; i < ARRAY_SIZE; ++i) {
      array[i] = i; // Populate array with something.
    }

    printf("[%d] Calling Isend\n", my_rank);

    MPI_Request request;
    MPI_Isend(array, ARRAY_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);

    int completed = 0;
    while (!completed) {
      // Do some work while waiting.
      usleep(100);
      printf(".");
      MPI_Test(&request, &completed, MPI_STATUS_IGNORE);
    }

    // Note: another option would be to use MPI_Wait, but MPI_Wait is blocking,
    // so it wouldn't work in a loop like this, only if you have *one* long
    // computation to perform before deciding to wait.

    printf("[%d] Done!\n", my_rank);
  }
  else {
    sleep(2);
    MPI_Recv(array, ARRAY_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[%d] Last element of array is: %d\n", my_rank,
           array[ARRAY_SIZE -1 ]);
  }

  free(array);
}

/**
 * NOTE: I didn't write the next three examples because they are *very* similar
 * to previous examples... Might do that later.
 */

/**
 * Example of MPI_Ibsend usage.
 * MPI_Ibsend is like MPI_Bsend, but it returns immediately.
 */
void ex_ibsend(int my_rank) {
  printf("[%d] Running example: MPI_Ibsend\n", my_rank);
}

/**
 * Example of MPI_Issend usage.
 * MPI_Issend is like MPI_Ssend, but it returns immediately. And MPI_Ssend is
 * like MPI_Send, but it's always synchronous. So the only difference between
 * this example and the MPI_Isend example is when the MPI_Test returns true.
 */
void ex_issend(int my_rank) {
  printf("[%d] Running example: MPI_Issend\n", my_rank);
}

/**
 * Example of MPI_Irsend usage.
 * MPI_Irsend is like MPI_Rsend, but it returns immediately.
 */
void ex_irsend(int my_rank) {
  printf("[%d] Running example: MPI_Irsend\n", my_rank);
}
