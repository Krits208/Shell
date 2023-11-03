#include "generics.h"
#include "globals.h"
#include "headers.h"

#include "init.h"
#include "prompt.h"
#include "shellinput.h"

int main() {
  int run = 1;

  init();

  char *homeDir = getHomeDir();

  while (run) {
    prompt();
    shellInput(homeDir);
    fflush(stdin);
    fflush(stdout);
    
  }

  return 0;
}
