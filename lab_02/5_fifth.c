#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int ABLE_TO_WRITE = 0;

void allow_handler(int sig_num) { ABLE_TO_WRITE = 1; }

int main(void) {
  int pipefd[2];
  pid_t childPIDS[2];

  char messages[2][32] = {"111111", "222222222222"};

  signal(SIGINT, allow_handler);

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
      if (!ABLE_TO_WRITE) {
        sleep(5);
        if (!ABLE_TO_WRITE)
          err(EXIT_FAILURE, "no signal");
      }
      write(pipefd[1], messages[i], strlen(messages[i]));
      if (close(pipefd[1]) == -1)
        err(EXIT_SUCCESS, "close");
      exit(EXIT_SUCCESS);
    }
  }

  for (size_t i = 0; i < 2; i++) {
    int status;
    waitpid(childPIDS[i], &status, 0);

    if (WIFEXITED(wstatus)) {
      printf("Child pid=%d exited, status=%d\n", cpid, WEXITSTATUS(wstatus));
    } else if (WIFSIGNALED(wstatus)) {
      printf("Child pid=%d killed by signal %d\n", cpid, WTERMSIG(wstatus));
    } else if (WIFSTOPPED(wstatus)) {
      printf("Child pid=%d stopped by signal %d\n", cpid, WSTOPSIG(wstatus));
    } else if (WIFCONTINUED(wstatus)) {
      printf("Child pid=%d continued\n", cpid);
    }
  }

  if (close(pipefd[1]) == -1)
    err(EXIT_FAILURE, "close");

  for (int i = 0; i < 2; i++) {
    char buf[256] = {0};
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