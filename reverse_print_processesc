#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
 
void cria_proc(int n)
{
  pid_t childpid;
  if (n == 0)
  {
    exit(0);
  }
  else
  {
    if ((childpid = fork()) == 0)
    {
      cria_proc(n-1);
      wait(NULL);
      printf("i am %ld and my father is %ld\n",(long)getpid(),(long)getppid());
      exit(0);
    }
  }
}
 
int main(void)
{ 
  int n=5;
  cria_proc(n);
  wait(NULL);
  printf("i'm %d, the original\n", getpid());
  
  return 0;
}
