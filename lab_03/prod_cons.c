#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ELEMENTS 32

#define SEMAPHORE_EMPTY 0
#define SEMAPHORE_FULL 1
#define SEMAPHORE_BINARY 2

#define CONSUMER_COUNT 3
#define PRODUCER_COUNT 3

#define p -1
#define v 1

int flag = 1;

struct sembuf start_consume[] = {{SEMAPHORE_FULL, p, 0},
                                 {SEMAPHORE_BINARY, p, 0}};
struct sembuf end_consume[] = {{SEMAPHORE_BINARY, v, 0},
                               {SEMAPHORE_EMPTY, v, 0}};

struct sembuf start_produce[] = {{SEMAPHORE_EMPTY, p, 0},
                                 {SEMAPHORE_BINARY, p, 0}};
struct sembuf end_produce[] = {{SEMAPHORE_BINARY, v, 0},
                               {SEMAPHORE_FULL, v, 0}};

void signal_handler(int signo, siginfo_t *info, void *context) {
  flag = 0;
  printf("Catched: PID %d signal %d\n", getpid(), signo);
}

void consumer(int semid, char *storage, char **consumep) {
  while (flag) {
    int err = semop(semid, start_consume, 2);
    if (err == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("semop\n");
      exit(1);
    }

    printf("%d get %c\n", getpid(), **consumep);

    (*consumep)++;
    if (*consumep - storage == ELEMENTS) {
      *consumep = storage;
    }

    if (semop(semid, end_consume, 2) == -1) {
      perror("semop\n");
      exit(1);
    }
  }
}
void producer(int semid, char *letter, char *storage, char **producep) {
  while (flag) {
    int err = semop(semid, start_produce, 2);
    if (err == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("semop\n");
      exit(1);
    }

    printf("%d put %c\n", getpid(), *letter);

    *((*producep)++) = *letter;
    (*letter)++;

    if (*letter == 'z' + 1)
      *letter = 'a';

    if (*producep - storage == ELEMENTS)
      *producep = storage;

    if (semop(semid, end_produce, 2) == -1) {
      perror("semop\n");
      exit(1);
    }
  }
}

int main() {
  key_t shmid, semid;
  struct sembuf sem_op;

  struct sigaction act = {0};
  act.sa_sigaction = &signal_handler;

  if (sigaction(SIGINT, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
  if (semid == -1) {
    perror("semget");
    err(EXIT_FAILURE, "semget");
  }

  if (semctl(semid, SEMAPHORE_EMPTY, SETVAL, ELEMENTS) == -1) {
    perror("semctl");
    return 1;
  }
  if (semctl(semid, SEMAPHORE_BINARY, SETVAL, 1) == -1) {
    perror("semctl");
    return 1;
  }
  if (semctl(semid, SEMAPHORE_FULL, SETVAL, 0) == -1) {
    perror("semctl");
    return 1;
  }

  shmid = shmget(IPC_PRIVATE, getpagesize(), IPC_CREAT | 0666);
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
      if (i % 2 == 0) {
        alarm(1);
        producer(semid, letter, storage, producep);
        exit(0);
      } else {
        alarm(1);
        consumer(semid, storage, consumep);
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

  if (semctl(semid, IPC_RMID, 0) == -1) {
    perror("semctl");
    return 1;
  }

  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    perror("shmctl");
    return 1;
  }

  return 0;
}
