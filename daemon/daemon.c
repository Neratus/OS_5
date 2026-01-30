#include "daemon.h"
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

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
