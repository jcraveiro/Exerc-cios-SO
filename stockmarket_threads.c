#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define NUM_WRITERS 5           // number of writers
#define NUM_READERS 5           // number of readers
#define STOCKLIST_SIZE 10  // stock list slots

typedef struct
{
  int readers;
  int slots[STOCKLIST_SIZE];
}mem_structure;

mem_structure *stocklist;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t *stop_writers;
int shmid;

pthread_t my_threads[NUM_WRITERS + NUM_READERS];
int id[NUM_WRITERS + NUM_READERS];

void init()
{
  sem_unlink("STOP_WRITERS");
  stop_writers = sem_open("STOP_WRITERS", O_CREAT|O_EXCL, 0700, 1);

  shmid = shmget(IPC_PRIVATE, sizeof(mem_structure), IPC_CREAT|0700);
  stocklist = (mem_structure*) shmat(shmid, NULL, 0);
  stocklist->readers = 0;
}

void terminate(int signo)
{
  pthread_mutex_destroy(&mutex);
  
  sem_close(stop_writers);
  sem_unlink("STOP_WRITERS");

  if (shmid >= 0)  // remove shared memory
  {
    shmctl(shmid, IPC_RMID, NULL);
  }
  
  printf("Closing...\n");
  exit(0);
}

int get_stock_value()
{
  return (1 + (int) (100.0 * rand() / (RAND_MAX + 1.0)));
}

int get_stock()
{
  return (int) (rand() % STOCKLIST_SIZE);
}

void write_stock(int n_writer)
{
  int stock = get_stock();
  int stock_value = get_stock_value();

  stocklist->slots[stock] = stock_value;
  fprintf(stderr, "Stock %d updated by BROKER %d to %d\n", stock, n_writer, stock_value);
}

void* writer_code(void* n_writer)
{
  int aux = *((int*) n_writer);
  int writer = aux;

  while(1)
  {
    pthread_mutex_lock(&mutex);
    sem_wait(stop_writers);

    write_stock(writer);

    sem_post(stop_writers);
    pthread_mutex_unlock(&mutex);

    sleep(1 + (int) (10.0 * rand() / (RAND_MAX + 1.0)));
  }
  pthread_exit(NULL);
}

void read_stock(int pos, int n_reader)
{
  fprintf(stderr, "Stock %d read by client %d = %d.\n", pos, n_reader, stocklist->slots[pos]);
}

void* reader_code(void* n_reader)
{
  int aux = *((int*) n_reader);
  int reader = aux;

  while(1)
  {
    
    sem_wait(stop_writers);
    read_stock(get_stock(), reader);
    sem_post(stop_writers);
   
    sleep(1 + (int) (3.0 * rand() / (RAND_MAX + 1.0)));
  }
  pthread_exit(NULL);
}

void monitor() // main process monitors the reception of Ctrl-C
{
  struct sigaction act;
  act.sa_handler = terminate;
  act.sa_flags = 0;
  if ((sigemptyset(&act.sa_mask) == -1) ||
      (sigaction(SIGINT, &act, NULL) == -1))
    perror("Failed to set SIGINT to handle Ctrl-C");
  while(1){
    sleep(5);
    printf("Still working...\n");
  }
  exit(0);
}

int main()
{
  int i = 0;

  init();

  for (i = 0; i < NUM_WRITERS + NUM_READERS; i++)
  {
    id[i] = i;
    

    if (i%2 == 0)
    {
      pthread_create(&my_threads[i], NULL, writer_code, &id[i]);
    }
    else
    {
      pthread_create(&my_threads[i], NULL, reader_code, &id[i]);
    }
  }

  monitor();
  exit(0);
}
