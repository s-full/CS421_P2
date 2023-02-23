#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 50

static char buffer[BUFFER_SIZE];

/* the signal handler function */
void handle_SIGTSTP() {
      write(STDOUT_FILENO,buffer,strlen(buffer));

      exit(0);
}

int main(int argc, char *argv[])
{
      /* set up the signal handler */
      struct sigaction handler;
      handler.sa_handler = handle_SIGTSTP;
      handler.sa_flags = SA_RESTART;
      sigaction(SIGTSTP, &handler, NULL);

      strcpy(buffer,"Caught <ctrl><z>\n");

      /* wait for <control> <Z> */
      while (1)
            ;

      return 0;
}
