#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int pipefd[2];
  pid_t childPIDS[2];

  char messages[2][32] = {"111111", "222222222222"};

  if (pipe(pipefd) == -1)
    err(EXIT_FAILURE, "pipe");

  for (int i = 0; i < 2; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("Can't fork");
      exit(1);
    } else if (childPIDS[i] == 0) {
      if (close(pipefd[0]) == -1)
        err(EXIT_FAILURE, "close");
      write(pipefd[1], messages[i], strlen(messages[i]));
      if (close(pipefd[1]) == -1)
        err(EXIT_FAILURE, "close");
      exit(EXIT_SUCCESS);
    }
  }

  for (size_t i = 0; i < 2; i++) {
    int status;
    waitpid(childPIDS[i], &status, 0);

    if (WIFEXITED(status)) {
      printf("Child (pid: %d) exited with code %d\n", childPIDS[i],
             WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("Child (pid: %d) received signal %d\n", childPIDS[i],
             WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("Child (pid: %d) received signal %d\n", childPIDS[i],
             WSTOPSIG(status));
    }
  }

  if (close(pipefd[1]) == -1)
    err(EXIT_FAILURE, "close");

  for (int i = 0; i < 3; i++) {
    char buf[256];
    int nbytes = read(pipefd[0], buf, strlen(messages[i]));
    printf("Read message: %s ; bytes: %d\n", buf, nbytes);
    if (nbytes == -1) {
      err(EXIT_FAILURE, "read");
    }
  }

  if (close(pipefd[0]) == -1)
    err(EXIT_FAILURE, "close");

  exit(EXIT_SUCCESS);
}