#include "definitions.h"
#include "globals.h"
#include "headers.h"

void cleanup() {
  printf("> Exiting . . .\n");
  char *homeDir = getHomeDir();
  free(homeDir);

  char **historyList = getHistoryList();
  free(historyList);

  processDetails *temp, *head;
  temp = head = getChildProcessList();
  while ((temp->next) != NULL) {
    processDetails *node = temp->next;
    
    head = temp;
    temp = temp->next;
    free(head);
  }
  free(head);
  printf("\n------------------------------------------------------------\n\n");
  exit(0);
}
