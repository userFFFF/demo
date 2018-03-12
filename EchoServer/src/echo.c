#include <stdlib.h>
#include <string.h>
#include "sync_server.h"
#include "asyn_server.h"
#include "cfg_reader.h"
#include "asyn_log.h"
#include "background.h"

int main() {
    KVPair *cfg = readFromFile("echo.cfg");

    char *run_mode = getValue(cfg, "run_mode");
    char *net_mode = getValue(cfg, "net_mode");
    char *listen_ip = getValue(cfg, "listen_ip");
    char *listen_port = getValue(cfg, "listen_port");
    char *log_directory = getValue(cfg, "log_directory");
    char *log_file_name = getValue(cfg, "log_file_name");
    char *log_file_size = getValue(cfg, "log_file_size");
    char *log_file_number = getValue(cfg, "log_file_number");

    if(strcmp(run_mode, "background") == 0)
        run_background();

    logInit(log_directory, log_file_name, atoi(log_file_size), atoi(log_file_number));

    if(strcmp(net_mode, "asyn") == 0)
        asyn_server(listen_ip, atoi(listen_port));
    else
        sync_server(listen_ip, atoi(listen_port));

    freeKVPair(cfg);
    return 0;
}