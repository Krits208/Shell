#include "childproc.h"
#include "definitions.h"
#include "globals.h"
#include "headers.h"


void execute(char **data, int len, char *trimmedCommand, int isBackground) {
  if (data != NULL && len > 0) {
    int pid = fork();
    if (pid > 0) {
      // parent
      setCurrentFgPid(pid);
      if (isBackground != 1) {
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        tcsetpgrp(STDIN_FILENO, pid);
        
        int status;
        if (waitpid(pid, &status, WUNTRACED) == -1) {
          perror("Error, process cannot finish");
         
          return;
        }
        if (WIFSTOPPED(status) != 0) {
          if (getCurrentFgPid() != -1) {
            processDetails *node = addProcNode(pid, data[0], trimmedCommand,
                                               getChildProcessList());
            printf("[%d] \n", node->pid);
          }
        }
        tcsetpgrp(STDIN_FILENO, getpgrp());
       
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        setCurrentFgPid(-1);
      } else {
        processDetails *node =
            addProcNode(pid, data[0], trimmedCommand, getChildProcessList());
        printf("[%d] %d\n",  node->pid);
      }
    } else if (pid == 0) {
      setpgid(0, 0);
      execvp(data[0], data);
      printf("%s: Command not found\n", data[0]);
      exit(0);
    } else {
      printf("Error: Process can not be created\n");
    }
  }
}
