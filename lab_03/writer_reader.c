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

#define WRITE_QUEUE 0
#define READ_QUEUE 1
#define ACTIVE_WRITER 2
#define ACTIVE_READERS 3

#define READERS_COUNT 3
#define WRITERS_COUNT 3

#define p -1
#define v 1

int flag = 1;

struct sembuf start_read[] = {{READ_QUEUE, v, 0},
                              {WRITE_QUEUE, 0, 0},
                              {ACTIVE_WRITER, 0, 0},
                              {ACTIVE_READERS, v, 0},
                              {READ_QUEUE, p, 0}};

struct sembuf end_read[] = {{ACTIVE_READERS, p, 0}};

struct sembuf start_write[] = {{WRITE_QUEUE, v, 0},
                               {ACTIVE_WRITER, 0, 0},
                               {ACTIVE_READERS, 0, 0},
                               {ACTIVE_WRITER, v, 0},
                               {WRITE_QUEUE, p, 0}};

struct sembuf end_write[] = {{ACTIVE_WRITER, p, 0}};

void signal_handler(int signo, siginfo_t *info, void *context) {
  flag = 0;
  printf("Catched: PID %d signal %d\n", getpid(), signo);
}

void reader(int semid, char *var) {
  while (flag) {
    int err = semop(semid, start_read, 5);
    if (err == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("semop\n");
      exit(1);
    }

    printf("%d reader %c\n", getpid(), *var);

    if (semop(semid, end_read, 1) == -1) {
      perror("semop\n");
      exit(1);
    }
  }
}

void writer(key_t semid, char *var) {
  while (flag) {
    int err = semop(semid, start_write, 5);
    if (err == -1) {
      if (errno == EINTR) {
        printf("PID %d caught interrupt on block\n", getpid());
        exit(1);
      }
      perror("semop\n");
      exit(1);
    }

    (*var)++;
    if (*var == 'z' + 1) {
      *var = 'a';
    }

    printf("%d writer %c\n", getpid(), *var);

    err = semop(semid, end_write, 1);
    if (err == -1) {
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

  semid = semget(IPC_PRIVATE, 4, IPC_CREAT | 0666);
  if (semid == -1) {
    perror("semget");
    err(EXIT_FAILURE, "semget");
  }

  if (semctl(semid, ACTIVE_READERS, SETVAL, 0) == -1) {
    perror("semctl");
    return 1;
  }
  if (semctl(semid, ACTIVE_WRITER, SETVAL, 0) == -1) {
    perror("semctl");
    return 1;
  }
  if (semctl(semid, WRITE_QUEUE, SETVAL, 0) == -1) {
    perror("semctl");
    return 1;
  }
  if (semctl(semid, READ_QUEUE, SETVAL, 0) == -1) {
    perror("semctl");
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

  *storage = 'a';

  pid_t childPIDS[READERS_COUNT + WRITERS_COUNT];
  for (int i = 0; i < READERS_COUNT + WRITERS_COUNT; i++) {
    childPIDS[i] = fork();
    if (childPIDS[i] == -1) {
      perror("fork");
      exit(1);
    } else if (childPIDS[i] == 0) {
      alarm(1);
      if (i % 2 == 0) {
        writer(semid, storage);
        exit(0);
      } else {
        reader(semid, storage);
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
