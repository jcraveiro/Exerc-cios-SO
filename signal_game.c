#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

int t_ini,num;
int t_end;

/* Create the function that handles CTRL-Z */
void sigtstp(int signum)
{
  char options[2];
  printf("\n ^Z pressed. Do you want to abort? ");
  scanf("%1s", options);
  if (options[0] == 'y')
  {
    printf("Ok, bye bye!\n");
    exit(0);
  }
}
/* function end */

/* Create the function that handles CTRL-C */
void sigint(int signum)
{
  int t_elapsed;
  t_end = time(NULL);
  t_elapsed = t_end - t_ini;

  if (t_elapsed == num)
  {
    printf("=> %ds elapsed! Great Shot!\n", t_elapsed);
  }
  else if (t_elapsed > num)
  {
    printf("=> %ds elapsed! You were too slow...\n", t_elapsed);
  }
  else
  {
    printf("=> %ds elapsed! You were too fast\n", t_elapsed);
  }
}
/* function end */

int main()
{
  /* Redirect signals */
  signal(SIGTSTP,sigtstp);
  // Do some work!
  srand(time(NULL));
  /* Ignore the SIGINT (CTRL-C) */
  signal(SIGINT, SIG_IGN);
  /* End ignore */
  
  while (1) {
	num=rand()%8+2;
	printf("\n\nPress CTRL-C in %d seconds!\n",num);
	sleep(2);
        printf("Countdown starting in 2 seconds... Get Ready!!\n");
        sleep(2);
	/* Redirects the SIGINT again */
        signal(SIGINT, sigint);
	/* End redirect */

	printf("Start counting the seconds!!\n");
	t_ini=time(NULL);
    	pause();
	}
  return 0;
}

