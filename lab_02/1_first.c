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
      sleep(2);
      printf("Child %d: PID = %d, PPID = %d, group = %d\n", i + 1, getpid(),
             getppid(), getpgrp());
      return 0;
    }
  }

  return 0;
}