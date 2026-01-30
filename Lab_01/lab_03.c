#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  const int child_cnt = 2;
  char *apps[2] = {"./app_01", "./app_02"};
  pid_t childPIDS[child_cnt];

  for (int i = 0; i < child_cnt; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("Can't fork");
      exit(1);
    }
    if (childPIDS[i] == 0) {
      printf("%d\n", getpid());
      if (execlp(apps[i], "args", NULL) == -1) {
        perror("Can't exec\n");
        exit(0);
      }
    }
  }

  int status;
  for (int i = 0; i < child_cnt; i++) {
    waitpid(childPIDS[i], &status, 0);
    if (WIFEXITED(status)) 
       printf("Потомок PID=%d завершён, статус=%d\n", childPIDS[i], WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
       printf("Потомок PID=%d завершён сигналом %d\n", childPIDS[i], WTERMSIG(status));
    else if (WIFSTOPPED(status))
       printf("Потомок PID=%d остановлен сигналом %d\n", childPIDS[i], WSTOPSIG(status));
    else if (WIFCONTINUED(status))
       printf("Потомок PID=%d продолжил работу\n", childPIDS[i]);
      
  }

  printf("Parent: PID = %d, group = %d, children = %d, %d\n", getpid(),
         getpgrp(), childPIDS[0], childPIDS[1]);

  exit(0);
}

