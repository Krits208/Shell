#include "generics.h"
#include "globals.h"
#include "headers.h"
#include "pinfo.h"

#include "cd.h"
#include "cleanup.h"
#include "echo.h"

#include "execute.h"
#include "fgbg.h"
#include "history.h"
#include "env.h"
#include "jobs.h"
#include "childproc.h"

#include "ls.h"

#include "pwd.h"

void executeCommand(char **parsed, int parsedLength, char *trimmedCommand) {
  int isBackground = 0;
  // printf("executeeeeee");
  if (strcmp(parsed[parsedLength - 1], "&") == 0) {
    isBackground = 1;
    parsed[parsedLength - 1] = NULL;
    parsedLength--;
    trimmedCommand = substr(trimmedCommand, 0, strlen(trimmedCommand) - 2);
  } else {
    parsed[parsedLength] = NULL;
  }

  if (parsedLength > 0) {
    char *homeDir = getHomeDir();
    char *command = parsed[0];

    saveCommand(command);

    if (strcmp(command, "cd") == 0) {
      cd(parsed[1], homeDir);
    } else if (strcmp(command, "pwd") == 0) {
      printf("%s\n", pwd());
    } else if (strcmp(command, "echo") == 0) {
      echo(parsed, parsedLength);
    } else if (strcmp(command, "ls") == 0) {
      ls(parsed, parsedLength, homeDir, "ls");
    } else if (strcmp(command, "la") == 0) {
      ls(parsed, parsedLength, homeDir, "la");
    } else if (strcmp(command, "ll") == 0) {
      ls(parsed, parsedLength, homeDir, "ll");
    } else if (strcmp(command, "pinfo") == 0) {
      pinfo(parsed, parsedLength, homeDir);
    } else if (strcmp(command, "history") == 0) {
      history(parsed, parsedLength);
    } else if (strcmp(command, "setenv") == 0) {
      setEnv(parsed, parsedLength);
    } else if (strcmp(command, "getenv") == 0) {
      getEnv(parsed, parsedLength);
    } else if (strcmp(command, "unsetenv") == 0) {
      unsetEnv(parsed, parsedLength);
    } else if (strcmp(command, "jobs") == 0) {
      jobs();
    } else if (strcmp(command, "kjob") == 0) {
      kjob(parsed, parsedLength);
    } else if (strcmp(command, "overkill") == 0) {
      overkill();
    } else if (strcmp(command, "fg") == 0) {
      fg(parsed, parsedLength);
    } else if (strcmp(command, "bg") == 0) {
      bg(parsed, parsedLength);
    } else if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
      cleanup();
    } else {
      execute(parsed, parsedLength, trimmedCommand, isBackground);
    }
  }
}

void runCommand(char *command) {
  // printf(" --->%s<---\n", command);
  char *trimmedCommand = trim(command, 0);

  char **parsed = (char **)malloc((sizeof(char) * 1024) * 1024);
  int parsedLength = split(trimmedCommand, ' ', parsed, 1024);

  if (parsedLength > 0 && parsed != NULL) {
    executeCommand(parsed, parsedLength, trimmedCommand);
  }

  free(parsed);
  // free(trimmedCommand);
}

void parseIORedir(char *command, int stdinBkp2, int stdoutBkp2) {
  char *trimmedCommand = trim(command, 0);
  // printf("-> %s -- ", trimmedCommand);
  char **parsed = (char **)malloc((sizeof(char) * 1024) * 1024);
  int parsedLength = split(trimmedCommand, ' ', parsed, 1024);

  int ipFlag, opFlag, opaFlag;
  ipFlag = opFlag = opaFlag = 0;
  int firstInd = parsedLength;

  int fin = 0, fop = 0;
  // char * input;

  for (int i = 0; i < parsedLength; i++) {
    if ((i == (parsedLength - 1)) && (strcmp(parsed[i], "&") == 0)) {
      continue;
    }
    if (strcmp(parsed[i], "<") == 0) {
      ipFlag = 1;
      opFlag = 0;
      opaFlag = 0;
      if (firstInd == parsedLength) {
        firstInd = i;
      }
    } else if (strcmp(parsed[i], ">") == 0) {
      ipFlag = 0;
      opFlag = 1;
      opaFlag = 0;
      if (firstInd == parsedLength) {
        firstInd = i;
      }
    } else if (strcmp(parsed[i], ">>") == 0) {
      ipFlag = 0;
      opFlag = 0;
      opaFlag = 1;
      if (firstInd == parsedLength) {
        firstInd = i;
      }
    } else {
      if (ipFlag) {
        if (fin != 0) {
          close(fin);
        }
        fin = open(parsed[i], O_RDONLY);
        if (fin == -1) {
          perror("Source file does not exist");
          return;
        }
      }
      if (opFlag || opaFlag) {
        if (fop != 0) {
          close(fop);
        }
        if (opaFlag) {
          fop = open(parsed[i], O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
          fop = open(parsed[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fop == -1) {
          perror("Error while opening file for writing");
          return;
        }
      }
    }
  }

  int stdinBkp, stdoutBkp;

  
  if (fin != 0) {
    
    if (dup2(fin, STDIN_FILENO) == -1) {
      perror("Error overwriting STDIN file descriptor");
      return;
    }
  }

  
  if (fop != 0) {
   
    if (dup2(fop, STDOUT_FILENO) == -1) {
      perror("Error overwriting STDOUT file descriptor");
      return;
    }
  }

  int len = 0;
  for (int i = 0; i < firstInd; i++) {
    if (i != (firstInd - 1)) {
      len += 1;
    }
    len += strlen(parsed[i]);
  }

  char *mainCommand = substr(trimmedCommand, 0, len);
  
  runCommand(mainCommand);

  

  if (fin != 0) {
    if (dup2(stdinBkp2, STDIN_FILENO) == -1) {
      perror("Error overwriting STDIN file descriptor");
      return;
    }
    close(fin);
  }

  if (fop != 0) {
    if (dup2(stdoutBkp2, STDOUT_FILENO) == -1) {
      perror("Error overwriting STDOUT file descriptor");
      return;
    }
    close(fop);
  }

  free(mainCommand);
  free(parsed);
  free(trimmedCommand);
}

void parsePipes2(char *command) {
  char **splitCommand = (char **)malloc((sizeof(char) * 1024) * 1024);
  int splitCommandLength = split(command, '|', splitCommand, 1024);

  int vfile[2];
  if (pipe(vfile) == -1) {
    perror("Error Creating Pipe");
    exit(1);
  }
  int childPid = fork();
  if (childPid == -1) {
    perror("Error Creating Fork");
    exit(1);
  } else if (childPid == 0) {
    
    if (dup2(vfile[0], STDOUT_FILENO) == -1) {
      perror("Error duplicating STDOUT file descriptor");
      exit(1);
    }
    runCommand(splitCommand[0]);
   
    close(vfile[1]);
  } else {
    close(vfile[1]);
    printf("parent\n");
    wait(NULL);

    char buf;
    while (read(vfile[0], &buf, 1) > 0)
      write(STDOUT_FILENO, &buf, 1);
    write(STDOUT_FILENO, "\n\n\n", 3);
    close(vfile[0]);
    exit(EXIT_SUCCESS);
  }

  
  exit(EXIT_SUCCESS);
}

void parsePipes(char *command) {
  char **splitCommand = (char **)malloc((sizeof(char) * 1024) * 1024);
  int splitCommandLength = split(command, '|', splitCommand, 1024);

  if (splitCommandLength > 0) {
    int stdinBkp = dup(STDIN_FILENO);
    if (stdinBkp == -1) {
      perror("Error duplicating STDIN file descriptor");
      return;
    }
    int stdoutBkp = dup(STDOUT_FILENO);
    if (stdoutBkp == -1) {
      perror("Error duplicating STDOUT file descriptor");
      return;
    }

    int pipeFile[splitCommandLength][2];
    for (int i = 0; i < splitCommandLength; i++) {
      if (pipe(pipeFile[i]) == -1) {
        perror("Error Creating Pipe");
        return;
      }
    }
    
    for (int i = 0; i < splitCommandLength - 1; i++) {
      if (i != 0) {
        close(pipeFile[i][1]);
        if (dup2(pipeFile[i][0], STDIN_FILENO) == -1) {
          perror("Error overwriting STDIN file descriptor");
          return;
        }
      }
      if (dup2(pipeFile[i + 1][1], STDOUT_FILENO) == -1) {
        perror("Error overwriting STDOUT file descriptor");
        return;
      }
      // runCommand(splitCommand[i]);
      if (i != 0) {
        parseIORedir(splitCommand[i], pipeFile[i][0], pipeFile[i + 1][1]);
        close(pipeFile[i][0]);
      } else {
        parseIORedir(splitCommand[i], stdinBkp, pipeFile[i + 1][1]);
      }
    }

    close(pipeFile[splitCommandLength - 1][1]);
    if (dup2(stdoutBkp, STDOUT_FILENO) == -1) {
      perror("Error overwriting STDOUT file descriptor");
      return;
    }
    if (splitCommandLength == 1) {
      if (dup2(stdinBkp, STDIN_FILENO) == -1) {
        perror("Error overwriting STDIN file descriptor");
        return;
      }
      parseIORedir(splitCommand[splitCommandLength - 1], stdinBkp, stdoutBkp);
    } else {
      // close(vfile[0]);
      if (dup2(pipeFile[splitCommandLength - 1][0], STDIN_FILENO) == -1) {
        perror("Error overwriting STDIN file descriptor");
        return;
      }
      parseIORedir(splitCommand[splitCommandLength - 1],
                   pipeFile[splitCommandLength - 1][0], stdoutBkp);
    }
    
    if (dup2(stdinBkp, STDIN_FILENO) == -1) {
      perror("Error overwriting STDIN file descriptor");
      return;
    }
    close(pipeFile[splitCommandLength - 1][0]);
  }
  free(splitCommand);
  
}

void parseInput(char *buffer) {
  char *trimmedBuffer = trim(buffer, 1);

  char **splitCommands = (char **)malloc((sizeof(char) * 1024) * 1024);
  int splitCommandsLength = split(trimmedBuffer, ';', splitCommands, 1024);

  for (int i = 0; i < splitCommandsLength; i++) {
    char *trimmedCommand = trim(splitCommands[i], 0);
    // runCommand(trimmedCommand);
    parsePipes(trimmedCommand);
    free(trimmedCommand);
  }

  free(splitCommands);
  free(trimmedBuffer);
}

void shellInput(char *homeDir) {
  int pid = getpid();

  char *buffer;
  size_t bufferLen = 1024;
  buffer = (char *)malloc(bufferLen * sizeof(char));

  if (getline(&buffer, &bufferLen, stdin) == EOF) {
    cleanup();
  }

  parseInput(buffer);

  free(buffer);
}
