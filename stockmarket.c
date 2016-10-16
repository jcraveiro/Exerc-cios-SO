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

#define NUM_WRITERS 5           // number of writers
#define NUM_READERS 5           // number of readers
#define STOCKLIST_SIZE 10  // stock list slots

typedef struct
{
  int readers;
  int slots[STOCKLIST_SIZE];
}mem_structure;

mem_structure *stocklist;
sem_t *mutex, *stop_writers;
int shmid;
pid_t childs[NUM_READERS + NUM_WRITERS];

void init()
{
  sem_unlink("MUTEX");
  mutex = sem_open("MUTEX", O_CREAT|O_EXCL,0700,1);

  sem_unlink("STOP_WRITERS");
  stop_writers = sem_open("STOP_WRITERS",  O_CREAT|O_EXCL, 0700,1);

  shmid = shmget(IPC_PRIVATE, sizeof(mem_structure), IPC_CREAT|0700);
  stocklist = (mem_structure*) shmat(shmid,NULL,0);
  stocklist->readers = 0;
}

void terminate(int signo)
{
  int i = 0;
  while (i < (NUM_READERS + NUM_WRITERS))
  {
    kill(childs[i++], SIGKILL);
  }
  while (wait(NULL) != -1);

  sem_close(mutex);
  sem_unlink("MUTEX");

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

int writer_code(int n_writer)
{
  srand(getpid());

  while(1)
  {
    sem_wait(stop_writers);
    sem_wait(mutex);

    write_stock(n_writer);

    sem_post(mutex);
    sem_post(stop_writers);

    sleep(1 + (int) (10.0 * rand() / (RAND_MAX + 1.0)));
  }

  exit(0);
}

void read_stock(int pos, int n_reader)
{
  fprintf(stderr, "Stock %d read by client %d = %d.\n", pos, n_reader, stocklist->slots[pos]);
}

int reader_code(int n_reader)
{
  srand(getpid());

  while(1)
  {
    sem_wait(mutex);
    stocklist->readers = stocklist->readers + 1;
    if (stocklist->readers == 1)
    {
      sem_wait(stop_writers);
    }
    sem_post(mutex);

    read_stock(get_stock(), n_reader);

    sem_wait(mutex);
    stocklist->readers = stocklist->readers - 1;
    if (stocklist->readers == 0)
    {
      sem_post(stop_writers);
    }
    sem_post(mutex);

    sleep(1 + (int) (3.0 * rand() / (RAND_MAX + 1.0)));
  }
  exit(0);
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
  int j = 0;

  init();

  while (i < NUM_WRITERS)
  {
    if ((childs[j] = fork()) == 0)
    {
      writer_code(i);
      exit(0);
    }
    else if (childs[j] == -1)
    {
      perror("Failed to create reader process");
      exit(1);
    }
    ++i;
    ++j;
  }
  i = 0;
  while (i < NUM_READERS)
  {
    if ((childs[j] = fork()) == 0) {
      reader_code(i);
      exit(0);
    }
    else if (childs[j] == -1) {
      perror("Failed to create writer process");
      exit(1);
    }
    ++i;
    ++j;
  }
  monitor();
  exit(0);
}
