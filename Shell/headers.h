#include <stdio.h>
#include <stdlib.h>

// for uname
#include <sys/utsname.h>

// for gethostname, getcwd, chdir
#include <unistd.h>

// for uid
#include <pwd.h>

// for strtok
#include <string.h>

// for PATH_MAX
#include <limits.h>

// for opendir
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

// for grgid
#include <grp.h>

// for strftime
#include <time.h>

// // for basename
// #include <libgen.h>

// for handling signals
#include <signal.h>

// for waitpid
#include <sys/wait.h>

// for open
#include <fcntl.h>

#define BUFFSIZE 4096
#define MAXHISTORY 20
#define HISTORYCOUNT 10
