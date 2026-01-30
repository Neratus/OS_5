#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  pid_t childPIDS[2];
  int i;

  for (i = 0; i < 2; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("Can't fork");
      exit(1);
    } else if (childPIDS[i] == 0) {
      printf("Child %d: PID = %d, PPID = %d, group = %d\n", i + 1, getpid(),
             getppid(), getpgrp());
      return 0;
    }
  }
  printf("Parent: PID = %d, group = %d, children = %d, %d\n", getpid(),
         getpgrp(), childPIDS[0], childPIDS[1]);
  int status;
  pid_t child_pid;

  for (i = 0; i < 2; i++) {
    child_pid = wait(&status);
    printf("Child has finished: PID = %d\n", child_pid);

    if (WIFEXITED(status))
      printf("Child exited with code %d\n", WEXITSTATUS(status));
    else
      printf("Child terminated abnormally\n");
  }

  return 0;
}
