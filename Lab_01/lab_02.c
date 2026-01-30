#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
  const int child_cnt = 2;
  pid_t childPIDS[child_cnt];

  for (int i = 0; i < child_cnt; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("Can't fork");
      exit(1);
    } else if (childPIDS[i] == 0) {
      printf("Child %d: PID = %d, PPID = %d, group = %d\n", i + 1, getpid(), getppid(), getpgrp());
      if (i == 0) {
      	sleep(30);
      }
      exit(0);
    }
  }
  printf("Parent: PID = %d, group = %d, children = %d, %d\n", getpid(),
         getpgrp(), childPIDS[0], childPIDS[1]);
  int status;
  pid_t child_pid;

  for (int i = 0; i < 2; i++) {
    child_pid = wait(&status);
 

    if (WIFEXITED(status)) 
       printf("Потомок PID=%d завершён, статус=%d\n", child_pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
       printf("Потомок PID=%d завершён сигналом %d\n", child_pid, WTERMSIG(status));
    else if (WIFSTOPPED(status))
       printf("Потомок PID=%d остановлен сигналом %d\n", child_pid, WSTOPSIG(status));
    else if (WIFCONTINUED(status))
       printf("Потомок PID=%d продолжил работу\n", child_pid);
 
  }

  exit(0);
}

