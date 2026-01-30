#include "lockdaemon.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#define LOCKFILE "/var/run/daemon.pid"

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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
    exit(1);
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
