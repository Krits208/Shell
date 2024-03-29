#include "childproc.h"
#include "definitions.h"
#include "globals.h"
#include "headers.h"

int terminalFG() {
  setCurrentFgPid(-1);
  tcsetpgrp(STDIN_FILENO, getpgrp());
  
  signal(SIGTTIN, SIG_DFL);
  signal(SIGTTOU, SIG_DFL);
  return 0;
}

int fg(char **parsedCommand, int parsedLength) {
  if (parsedLength == 2) {
    int jobId = atoi(parsedCommand[1]);

    processDetails *childProcessList = getChildProcessList();
    processDetails *process = childProcessList->next;

    int found = 0;
    while (process != NULL) {
      if (process->jobid == jobId) {
        found = 1;
        break;
      }
      process = process->next;
    }
    if (found) {
      processDetails *tmpProcess = generateProcNode(
          process->pid, process->pname, process->command, process->jobid);

      signal(SIGTTIN, SIG_IGN);
      signal(SIGTTOU, SIG_IGN);

      tcsetpgrp(STDIN_FILENO, tmpProcess->pid);
      // if (tcsetpgrp(STDIN_FILENO, tmpProcess->pid) == -1)
      // {
      //     perror("Error, process cannot be made foreground");
      //     return -1;
      // }
      if (kill(tmpProcess->pid, SIGCONT) == -1) {
        perror("Error, SIGCONT signal cannot be sent to the process");
        terminalFG();
        return -1;
      }

      setCurrentFgPid(tmpProcess->pid);
      deleteProcNode(process->pid, getChildProcessList());
      // wait for foreground process to finish
      int status;
      if (waitpid(tmpProcess->pid, &status, WUNTRACED) == -1) {
        perror("Error, process cannot finish");
        terminalFG();
        return -1;
      }
      if (WIFSTOPPED(status) != 0) {
        if (getCurrentFgPid() != -1) {
          processDetails *node =
              addProcNode(tmpProcess->pid, tmpProcess->pname,
                          tmpProcess->command, getChildProcessList());
          printf("[%d] %d\n", node->jobid, node->pid);
        }
      }

      // recover terminal
      free(tmpProcess);
      return terminalFG();
    } else {
      printf("Error: Job id does not exist\n");
      return -1;
    }
  } else {
    printf("Unsufficient arguments supplied\n");
    return -1;
  }
}

int bg(char **parsedCommand, int parsedLength) {
  if (parsedLength == 2) {
    int jobId = atoi(parsedCommand[1]);

    processDetails *childProcessList = getChildProcessList();
    processDetails *process = childProcessList->next;

    int found = 0;
    while (process != NULL) {
      if (process->jobid == jobId) {
        found = 1;
        break;
      }
      process = process->next;
    }
    if (found) {
      kill(process->pid, SIGCONT);
      return 0;
    } else {
      printf("Error: Job id does not exist\n");
      return -1;
    }
  } else {
    printf("Unsufficient arguments supplied\n");
    return -1;
  }
}
