#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include "sync_server.h"
#include "asyn_server.h"
#include "cfg_reader.h"
#include "asyn_log.h"
#include "background.h"
#include "single.h"

char run_mode[16];
char net_mode[16];
char listen_ip[16];
int listen_port;
char log_directory[PATH_MAX];
char log_file_name[NAME_MAX];
int log_file_size;
int log_file_number;

void readconf()
{
    KVPair *cfg  = readFromFile("echo.cfg");
    strcpy(run_mode, getValue(cfg, "run_mode"));
    strcpy(net_mode, getValue(cfg, "net_mode"));
    strcpy(listen_ip, getValue(cfg, "listen_ip"));
    listen_port = atoi(getValue(cfg, "listen_port"));
    strcpy(log_directory, getValue(cfg, "log_directory"));
    strcpy(log_file_name, getValue(cfg, "log_file_name"));
    log_file_size = atoi(getValue(cfg, "log_file_size"));
    log_file_number = atoi(getValue(cfg, "log_file_number"));
    freeKVPair(cfg);
}

void start()
{
    if(strcmp(net_mode, "asyn") == 0)
        asyn_server(listen_ip, listen_port);
    else
        sync_server(listen_ip, listen_port);
}

void stop()
{
    if(strcmp(net_mode, "asyn") == 0)
        stop_asyn_server();
    else
        stop_sync_server();
}

void *sigint(int signo)
{
    stop();
    logUninit();
    syslog(LOG_INFO, "got SIGINT; exiting");
    exit(0);
}

void *sigterm(int signo)
{
    stop();
    logUninit();
    syslog(LOG_INFO, "got SIGTERM; exiting");
    exit(0);
}

void *sighup(int signo)
{
    syslog(LOG_INFO, "Re-reading configuration file");
    stop();
    logUninit();
    readconf();
    logInit(log_directory, log_file_name, log_file_size, log_file_number);
    start();
}

int main(int argc, char *argv[]) {

    readconf();

    logInit(log_directory, log_file_name, log_file_size, log_file_number);

    if (strcmp(run_mode, "background") == 0) {
        char *cmd;
        if((cmd = strrchr(argv[0], '/')) == NULL)
            cmd = argv[0];
        else
            cmd++;

        daemonize(cmd);
    }

    if(already_running("echo")) {
        syslog(LOG_ERR, "echo already running");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGTERM, &sa, NULL) < 0) {
        syslog(LOG_ERR, "can't catch SIGTERM: %s", strerror(errno));
        exit(1);
    }

    sa.sa_handler = sighup;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0) {
        syslog(LOG_ERR, "can't catch SIGHUP: %s", strerror(errno));
        exit(1);
    }

    sa.sa_handler = sigint;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGHUP);
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        syslog(LOG_ERR, "can't catch SIGINT: %s", strerror(errno));
        exit(1);
    }

    start();

    return 0;
}