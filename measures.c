#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define PIPE_NAME   "my_pipe"

char str[256];
int fd_named_pipe,fd_temp[2],fd_hum[2]; // to save file descriptors
int temp_sum, temp_samples, hum_sum, hum_samples; // to save temperature and humidity sum and number of samples
int value;
int pid[2]; // saves the pid of child processes

// cleans all resources including closing of files
/* TO COMPLETE */
void clean_resources()
{
  close(fd_named_pipe);
  close(fd_hum[0]);
  close(fd_hum[1]);
  close(fd_temp[0]);
  close(fd_temp[1]);
  unlink(PIPE_NAME);
}

void shutdown_all()
{
  kill(pid[0], SIGKILL);
  kill(pid[1], SIGKILL);
  while (wait(NULL) != -1);
  clean_resources();
  printf("Bye bye\n");
  exit(0);
}

// handling of CTRL-C
/* TO COMPLETE */
void sigint(int signum)
{
  clean_resources();
  exit(0);
}

// generates temperature values and writes them in the corresponding pipe
/* TO COMPLETE */
void measure_temp()
{
  srand(getpid());

  close(fd_hum[0]);
  close(fd_hum[1]);
  close(fd_temp[0]); //fechar descriptor de leitura

  while(1)
  {
    int n = 10 + rand() % 30;
    write(fd_temp[1], &n, sizeof(int));
    sleep(3);
  }
}

/* TO COMPLETE */
void measure_humidity()
{
  srand(getpid());

  close(fd_temp[0]);
  close(fd_temp[1]);
  close(fd_hum[0]);

  while(1)
  {
    int n = 50 + rand() % 50;
    write(fd_hum[1], &n, sizeof(int));
    sleep(3);
  }
}


int main()
{
  int number_of_chars;
  fd_set read_set;

  // Server terminates when CTRL-C is pressed
  // Redirect CTRL-C
  /* TO COMPLETE */
  signal(SIGINT, sigint);

  //create pipes
  /* TO COMPLETE */
  pipe(fd_hum);
  pipe(fd_temp);

  //create childs
  if((pid[0]=fork())==0){
    measure_temp();
    exit(0);
  }
  if((pid[1]=fork())==0){
    measure_humidity();
    exit(0);
  }

  // Creates the named pipe if it doesn't exist yet
  /* TO COMPLETE */
  if ((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST))
  {
    perror("Cannot create pipe: ");
    exit(0);
  }

  // Opens the pipe for reading
  /* TO COMPLETE */
  if ((fd_named_pipe = open(PIPE_NAME, O_RDWR)) < 0)
  {
    perror("Cannot open pipe for reading: ");
    exit(0);
  }

  //close unneeded fd
  close(fd_temp[1]);
  close(fd_hum[1]);

  // Listen to messages

  printf("Listening to all pipes!\n\n");
  while (1)
  {

    // I/O Multiplexing
    /* TO COMPLETE */
    FD_ZERO(&read_set);
    FD_SET(fd_temp[0], &read_set);
    FD_SET(fd_hum[0], &read_set);
    FD_SET(fd_named_pipe, &read_set);

    if(select(fd_named_pipe + 1, &read_set, NULL, NULL, NULL) > 0)
    {

      if(FD_ISSET(fd_named_pipe,&read_set)){
        number_of_chars=read(fd_named_pipe, str, sizeof(str));
        str[number_of_chars-1]='\0'; //put a \0 in the end of string

        if(strcmp(str,"AVG TEMP")==0)
          printf("[SERVER Received \"%s\" command]\nAverage Temperature= %.2fºC\n", str,(double)temp_sum/temp_samples);
        else
          if(strcmp(str,"AVG HUM")==0)
            printf("[SERVER Received \"%s\" command]\nAverage Humidity= %.2f %%\n", str,(double)hum_sum/hum_samples);
          else
            if(strcmp(str,"RESET")==0){
              printf("[SERVER received \"%s\" command]\nCounters reset!\n", str);
              temp_sum=0;
              temp_samples=0;
              hum_sum=0;
              hum_samples=0;
            }
            else
              if(strcmp(str,"SHUTDOWN")==0){
                printf("[SERVER received \"%s\" command]\nServer shutdown initiated!\n", str);
                shutdown_all();
              }
              else
                printf("[SERVER Received unknown command]: %s \n",str);
      } // if(FD_ISSET(fd_named_pipe))
      if(FD_ISSET(fd_temp[0],&read_set)){
        read(fd_temp[0], &value, sizeof(int));
        printf("[SERVER received new temperature]: %dºC\n", value);
        temp_sum+=value;
        temp_samples++;
      }
      if(FD_ISSET(fd_hum[0],&read_set)){
        read(fd_hum[0], &value, sizeof(int));
        printf("[SERVER received new humidity]: %d %% \n", value);
        hum_sum+=value;
        hum_samples++;
      }
      /* TO COMPLETE */
    }
  } // While(1)
  return 0;
}
