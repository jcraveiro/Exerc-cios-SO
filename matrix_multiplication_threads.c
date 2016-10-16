#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define M 3 //first matrix lines
#define K 2 //first matrix columns; second matrix lines
#define N 3 //second matrix columns

struct v {
  int row;
  int column;
};

typedef struct v coordenate;
void *worker(void* coord);
void *show_matrix();    // prints matrices on screen

pthread_t my_thread[(M*N)+1];
int A[M][K] = { {1,4}, {2,5}, {3,6} };
int B[K][N] = { {8,7,6}, {5,4,3} };
int C[M][N];

// creates mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// prints matrices on screen when having exclusive access
void print_matrix()
{
  int i;
  int j;
  pthread_mutex_lock(&mutex);

  printf("\n");
  for (i = 0; i < M; i++)
  {
    for (j = 0; j < N; j++)
    {
      printf(" %d ", C[i][j]);
    }
    printf("\n");
  }
  printf("\n");

  pthread_mutex_unlock(&mutex);
}

//calculates each matrix element in the coordenate received
void *worker( void* coord)
{
  int sum = 0;
  coordenate coords = *((coordenate*) coord);
  int i;


  pthread_mutex_lock(&mutex);

  printf("This thread is working in C[%d][%d]\n",coords.row, coords.column);
  
  for (i = 0; i < K; i++)
  {
    sum += A[coords.row][i] * B[i][coords.column];
  }
  C[coords.row][coords.column] = sum;

  pthread_mutex_unlock(&mutex);
}

// prints the status of the matrix in each second, along a period of 10 seconds
void *show_matrix()
{
  int i;
  for (i = 0; i < 10; i++)
  {
    print_matrix();
    sleep(1);
  }
  pthread_exit(NULL);
}

int main()
{
  // creates threads and sends each the coordinates of the element to calculate
  int id = 0;
  coordenate* coords; 

  pthread_create(&my_thread[id], NULL, show_matrix, NULL);
  id++;
  
  for (int i = 0; i < M; i++){
    for (int j = 0; j < N; j++){
      coords = (coordenate*) malloc(sizeof(coordenate));
      coords->row = i;
      coords->column = j;
      pthread_create(&my_thread[id], NULL, worker, coords);
      id++;
      sleep(1);
    }
  }

  // waits for threads to finish
  for (int k = 0; k < M*N; k++) {
    pthread_join(my_thread[k], NULL);
  }
  
  free(coords);
  
  // prints final results of the calculation and leaves
  printf("Let's see the final result\n");
  print_matrix();
  
  //destroys mutex and exits
  pthread_mutex_destroy(&mutex);
  return 0;
}
