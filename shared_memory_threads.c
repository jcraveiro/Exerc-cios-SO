#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <time.h>

#define  N_WORKERS 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int *value;
int shmid;

void init()
{
  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0700);
  value = (int*) shmat(shmid, NULL, 0);
  *value = 1000;
}

void terminate()
{
  pthread_mutex_destroy(&mutex);
  
  if (shmid > 0)
  {
    shmctl(shmid, IPC_RMID, NULL);
  }
}

void *worker(void *id)
{
  int r;
  int aux = *((int*) id);
  int my_id = aux;

  srand(time(NULL));
  r = rand() % 2 + 1;
  sleep(r);

  pthread_mutex_lock(&mutex);
  
  *value += 1;
  printf("Thread with id == %d incremented value by 1\n", my_id);

  pthread_mutex_unlock(&mutex);
  
  pthread_exit(NULL);
}

int main()
{
  int i;
  
  init();
  
  pthread_t threads;
  int id[N_WORKERS];
  
  for (i = 0; i < N_WORKERS; i++)
  {
    id[i] = i;
    pthread_create(&threads, NULL, worker, &id[i]);
  }

  for (i = 0; i < N_WORKERS; i++)
  {
    pthread_join(threads, NULL);
  }

  printf("The final value is %d\n", *value);

  terminate();
}
