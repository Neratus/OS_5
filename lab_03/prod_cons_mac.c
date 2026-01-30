#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ELEMENTS 32

#define SEMAPHORE_EMPTY 0
#define SEMAPHORE_FULL 1
#define SEMAPHORE_BINARY 2

#define CONSUMER_COUNT 3
#define PRODUCER_COUNT 2

#define p -1
#define v 1

int flag = 1;

sem_t *sem_empty;
sem_t *sem_full;
sem_t *sem_binary;

void signal_handler(int signo, siginfo_t *info, void *context) {
  flag = 0;
  printf("Catched: PID %d signal %d\n", getpid(), signo);
}

void consumer(int semid, char *storage, char **consumep) {
  while (flag) {
    if (sem_wait(sem_full) == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("sem_wait\n");
      exit(1);
    }
    if (sem_wait(sem_binary) == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("sem_wait\n");
      exit(1);
    }

    printf("%d get %c\n", getpid(), **consumep);

    (*consumep)++;
    if (*consumep - storage == ELEMENTS) {
      *consumep = storage;
    }

    if (sem_post(sem_binary) == -1) {
      perror("sem_post\n");
      exit(1);
    }
    if (sem_post(sem_empty) == -1) {
      perror("sem_post\n");
      exit(1);
    }
  }
}

void producer(int semid, char *letter, char *storage, char **producep) {
  while (flag) {
    if (sem_wait(sem_empty) == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("sem_wait\n");
      exit(1);
    }
    if (sem_wait(sem_binary) == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("sem_wait\n");
      exit(1);
    }

    printf("%d put %c\n", getpid(), *letter);

    *((*producep)++) = *letter;
    (*letter)++;

    if (*letter == 'z' + 1)
      *letter = 'a';

    if (*producep - storage == ELEMENTS)
      *producep = storage;

    if (sem_post(sem_binary) == -1) {
      perror("sem_post\n");
      exit(1);
    }
    if (sem_post(sem_full) == -1) {
      perror("sem_post\n");
      exit(1);
    }
  }
}

int main() {
  key_t shmid;
  struct sigaction act = {0};
  act.sa_sigaction = &signal_handler;

  if (sigaction(SIGINT, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  sem_empty = sem_open("/empty", O_CREAT | O_EXCL, 0666, ELEMENTS);
  sem_full = sem_open("/full", O_CREAT | O_EXCL, 0666, 0);
  sem_binary = sem_open("/binary", O_CREAT | O_EXCL, 0666, 1);

  if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED ||
      sem_binary == SEM_FAILED) {
    perror("sem_open");
    return 1;
  }

  shmid = shmget(IPC_PRIVATE, 256, IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    return 1;
  }

  char *storage = shmat(shmid, NULL, 0);
  if (storage == (void *)-1) {
    perror("shmat");
    return 1;
  }

  char *letter = storage;
  *letter = 'a';
  storage++;

  char **producep = (char **)storage;
  storage += sizeof(char *);
  char **consumep = (char **)storage;
  storage += sizeof(char *);
  *producep = storage;
  *consumep = storage;

  pid_t childPIDS[CONSUMER_COUNT + PRODUCER_COUNT];
  for (int i = 0; i < CONSUMER_COUNT + PRODUCER_COUNT; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("fork");
      exit(1);
    } else if (childPIDS[i] == 0) {
      alarm(1);
      if (i % 2 == 0) {
        producer(0, letter, storage, producep);
        exit(0);
      } else {
        consumer(0, storage, consumep);
        exit(0);
      }
    }
  }

  pid_t wpid = 0;
  int wstatus = 0;
  while (wpid != -1) {
    wpid = wait(&wstatus);
    if (wpid == -1) {
      if (errno == EINTR) {
        wpid = 0;
      } else if (errno != ECHILD) {
        perror("wait");
        exit(EXIT_FAILURE);
      }
    } else {
      if (WIFEXITED(wstatus)) {
        printf("Child pid=%d exited, status=%d\n", wpid, WEXITSTATUS(wstatus));
      } else if (WIFSIGNALED(wstatus)) {
        printf("Child pid=%d killed by signal %d\n", wpid, WTERMSIG(wstatus));
      } else if (WIFSTOPPED(wstatus)) {
        printf("Child pid=%d stopped by signal %d\n", wpid, WSTOPSIG(wstatus));
      } else if (WIFCONTINUED(wstatus)) {
        printf("Child pid=%d continued\n", wpid);
      }
    }
  }

  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    perror("shmctl");
    return 1;
  }

  sem_close(sem_empty);
  sem_close(sem_full);
  sem_close(sem_binary);
  sem_unlink("/empty");
  sem_unlink("/full");
  sem_unlink("/binary");

  return 0;
}