#include "generics.h"
#include "globals.h"
#include "headers.h"
#include "pwd.h"

void prompt() {
  char *username = getCurrentUser();
  char *hostname = getHostName();

  char *path = rootPathResolve(pwd(), getHomeDir());

  printf("<%s@%s:%s> ", username, hostname, path);

  free(path);
}
