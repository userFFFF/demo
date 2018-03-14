//
// Created by wu on 18-3-9.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>

void run_background() {
    pid_t pid;

    if ((pid = fork()) < 0) {
        printf("fork error\n");
        exit(1);
    } else if (pid == 0) {
        printf("run background\n");
    } else {
        exit(1);
    }
}

void daemonize(const char *cmd)
{
    umask(0);

    struct rlimit rl;
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("%s: can't get file limit\n", cmd);
        exit(1);
    }

    pid_t pid;
    if((pid = fork()) < 0) {
        printf("%s: can't fork\n", cmd);
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    setsid();

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("%s: can't ignore SIGHUP\n", cmd);
        exit(1);
    }

    if((pid = fork()) < 0) {
        printf("%s: can't fork\n", cmd);
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    if(chdir("/") < 0) {
        printf("%s: can't change directory to /\n", cmd);
        exit(1);
    }

    if(rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    for (int i = 0; i < rl.rlim_max; ++i) {
        close(i);
    }

    int fd0 = open("dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);

    if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}