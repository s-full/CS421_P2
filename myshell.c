#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

#define BUFFER_SIZE 50

static char buffer[BUFFER_SIZE];
pid_t fg_pid = 0;

void handle_SIGQUIT() {
    write(STDOUT_FILENO,buffer,strlen(buffer));
    if (fg_pid != 0) {
        kill(fg_pid, SIGQUIT);

    }
    //exit(0);
}

void printWelcome(int currpid) {
	printf("Welcome to sfshell. My pid is %d. \n", currpid);
}
void printPrompt(int* count) {
    printf("sfshell[%d]: \n", *count);
    (*count)++;
}
 
/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
   
    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               /* argument separators */
            if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
          case '\n':                 /* should be the final char examined */
            if (start != -1){
                    args[ct] = &inputBuffer[start];    
                ct++;
            }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
            break;
          default :             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }   
     args[ct] = NULL; /* just in case the input line was > 80 */
}


int main(void)
{
    char inputBuffer[MAX_LINE];      /* buffer to hold the command entered */
    int background;              /* equals 1 if a command is followed by '&' */
    char *args[(MAX_LINE/2)+1];  /* command line (of 80) has max of 40 arguments */
    int promptCount = 1;
    
    pid_t pid;
    
    int pidt = getpid();

    // print introduction/welcome
    printWelcome(pidt);

    // this will handle ctrl+<\> 
    struct sigaction handler;
    handler.sa_handler = handle_SIGQUIT;
    handler.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &handler, NULL);
    strcpy(buffer,"Caught <ctrl><\\>\n");
    
    while (1){            /* Program terminates normally inside setup */
       background = 0;

       // print command promp and count
       printPrompt(&promptCount);
        setup(inputBuffer,args,&background);

        // built in commands

        if (!strcmp("bg", args[0])) {
            pid_t bg_pid = atoi(args[1]);
            kill(bg_pid, SIGCONT);
            fg_pid = 0;
            continue;
        }
        if (!strcmp("stop", args[0])) {
            pid_t stop_pid = atoi(args[1]);
            kill(stop_pid, SIGSTOP);
            continue;
        }
        if (!strcmp("fg", args[0])) {
            pid_t npid = atoi(args[1]);
            
            kill(npid, SIGCONT);
            
            continue;
            
        }
        if (!strcmp("kill", args[0])) {
            pid_t kill_pid = atoi(args[1]);
            kill(kill_pid, SIGKILL);
            continue;

        } 
        if (!strcmp("exit", args[0])){
            break;
        }
        
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed");
            return 1;
        }
        else if (pid == 0) {
            printf("[Child pid = %d, background = ", getpid());
            if (background) {
                printf("TRUE]\n");
                //setsid();
            }
            else {
                 printf("FALSE]\n");
            }
            execvp(args[0], args);
        }
        else {
            if (!background) {
                fg_pid = pid;
                waitpid(pid, NULL, WUNTRACED);
                printf("Child process complete\n");
                fg_pid = 0;
            }
                
                //waitpid(pid, NULL, 0);
                
        }
      /* the steps are:
       (0) if built-in command, handle internally
       (1) if not, fork a child process using fork()
       (2) the child process will invoke execvp()
       (3) if background == 0, the parent will wait,
            otherwise returns to the setup() function. */       
    }
    //kill(pid, SIGKILL);
    printf("sfshell exiting\n");
    return 0;
}
