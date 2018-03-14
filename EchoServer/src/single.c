//
// Created by wu on 18-3-13.
//

#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return fcntl(fd, F_SETLK, &fl);
}

int already_running(char *name)
{
    char lockFile[PATH_MAX];
    sprintf(lockFile, "/var/run/%s.pid", name);

    int fd = open(lockFile, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        syslog(LOG_ERR, "can't open %s: %s", lockFile, strerror(errno));
        exit(1);
    }

    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "can't lock %s: %s", lockFile, strerror(errno));
        exit(1);
    }

    char buf[16];
    sprintf(buf, "%ld", (long)getpid());
    ftruncate(fd, 0);
    write(fd, buf, strlen(buf)+1);
    return 0;
}