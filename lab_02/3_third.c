#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  const int child_cnt = 2;
  char *apps[child_cnt] = {"./a1", "./a2"};
  pid_t childPIDS[child_cnt];

  for (i = 0; i < child_cnt; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("Can't fork");
      exit(1);
    }
    if (childPIDS[i] == 0) {
      if (execlp(apps[i], "args", NULL) == -1) {
        perror("Can't exec\n");
        exit(1);
      }
    }
  }

  int status;
  for (i = 0; i < child_cnt; i++) {
    waitpid(childPIDS[i], &status, 0);
    if (WIFEXITED(status))
      printf("\t%d-Child exited with status = %d\n", childPIDS[i],
             WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      printf("\t%d-Child with (signal %d) killed\n", childPIDS[i],
             WTERMSIG(status));
    else if (WIFSTOPPED(status))
      printf("\t%d-Child with (signal %d) stopped\n", childPIDS[i],
             WSTOPSIG(status));
    else
      printf("\t%d-Unexpected status for Child (0x%x)\n", childPIDS[i], status);
  }

  printf("Parent: PID = %d, group = %d, children = %d, %d\n", getpid(),
         getpgrp(), childPIDS[0], childPIDS[1]);

  exit(0);
}