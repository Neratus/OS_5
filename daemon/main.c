#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define LOCKFILE "/var/run/daemon.pid"
#define CONFFILE "/etc/my_daemon.conf"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;

int lockfile(int fd) {
  struct flock f;
  f.l_type = F_RDLCK | F_WRLCK;
  f.l_start = 0;
  f.l_whence = SEEK_SET;
  f.l_len = 0;
  return (fcntl(fd, F_SETLK, &f));
}

int already_running(void) {
  int fd;
  char buf[16];
  fd = open(LOCKFILE, O_CREAT | O_EXCL | O_RDWR, LOCKMODE);
  if (fd < 0) {
    syslog(LOG_ERR, "open");
    return 1;
  }
  if (lockfile(fd) < 0) {
    syslog(LOG_ERR, "невозможно установить блокировку на %s", LOCKFILE);
    return 1;
  }
  ftruncate(fd, 0);
  sprintf(buf, "%ld", (long)getpid());
  write(fd, buf, strlen(buf) + 1);
  return (0);
}

void daemonize(const char *cmd) {
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;
  /*
   * Сбросить маску режима создания файла.
   */
  if (umask(0) == -1) {
    perror("umask");
    exit(1);
  }
  /*
   * Получить максимально возможный номер дескриптора файла.
   */
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
    perror("getrlimit");
    exit(1);
  }
  /*
   * Стать лидером нового сеанса, чтобы утратить управляющий терминал.
   */
  if ((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid > 0) /* родительский процесс */
    exit(0);

  if (setsid() == -1) {
    perror("setsid");
    exit(1);
  }
  /*
   * Обеспечить невозможность обретения управляющего терминала в будущем.
   */
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0) {
    perror("sigaction");
    exit(1);
  }
  /*
   * Назначить корневой каталог текущим рабочим каталогом,
   * чтобы впоследствии можно было отмонтировать файловую систему.
   */
  if (chdir("/") < 0) {
    perror("chdir");
    exit(1);
  }
  /*
   * Закрыть все открытые файловые дескрипторы.
   */
  if (rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;
  for (i = 0; i < rl.rlim_max; i++)
    close(i);
  /*
   * Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
   */
  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);
  /*
   * Инициализировать файл журнала.
   */
  openlog(cmd, LOG_CONS, LOG_DAEMON);
  if (fd0 != 0) {
    syslog(LOG_ERR, "stdin");
    exit(1);
  }
  if (fd1 != 1) {
    syslog(LOG_ERR, "stdout");
    exit(1);
  }
  if (fd2 != 2) {
    syslog(LOG_ERR, "stderr");
    exit(1);
  }
}

void reread(void) {
  FILE *fd;
  char data[256];

  if ((fd = fopen(CONFFILE, "r")) == NULL) {
    syslog(LOG_ERR, CONFFILE);
    exit(1);
  }
  fscanf(fd, "%s", data);
  fclose(fd);
}

void *thr_fn(void *arg) {
  int err, signo;
  for (;;) {
    err = sigwait(&mask, &signo);
    if (err != 0) {
      syslog(LOG_ERR, "ошибка вызова функции sigwait");
      exit(1);
    }
    switch (signo) {
    case SIGHUP:
      syslog(LOG_INFO, "получен сигнал SIGHUP");
      reread();
      break;
    case SIGTERM:
      syslog(LOG_INFO, "получен сигнал SIGTERM");
      pthread_exit(0);
    default:
      syslog(LOG_INFO, "получен сигнал %d\n", signo);
    }
  }
  return (0);
}

int main(int argc, char *argv[]) {
  int err;
  pthread_t tid;
  char *cmd;
  struct sigaction sa;
  if ((cmd = strrchr(argv[0], '/')) == NULL)
    cmd = argv[0];
  else
    cmd++;

  /*
   * Перейти в режим демона.
   */
  daemonize(cmd);
  /*
   * Убедиться, что ранее не была запущена другая копия демона.
   */

  if (already_running()) {
    syslog(LOG_ERR, "демон уже запущен");
    exit(1);
  }
  /*
   * Восстановить действие по умолчанию для сигнала SIGHUP
   * и заблокировать все сигналы.
   */
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0) {
    syslog(LOG_ERR, "%s: невозможно восстановить действие SIG_DFL для SIGHUP",
           cmd);
    exit(1);
  }

  sigfillset(&mask);
  if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
    syslog(LOG_ERR, "%s: ошибка выполнения операции SIG_BLOCK", cmd);
    exit(1);
  }
  /*
   * Создать поток для обработки SIGHUP и SIGTERM.
   */
  err = pthread_create(&tid, NULL, thr_fn, 0);
  if (err != 0) {
    syslog(LOG_ERR, "%s: ошибка создания потока", cmd);
    exit(1);
  }
  time_t raw_time;
  struct tm *timeinfo;

  for (;;) {
    sleep(2);
    time(&raw_time);
    timeinfo = localtime(&raw_time);
    syslog(LOG_INFO, "текущее время: %s", asctime(timeinfo));
  }
  exit(0);
}