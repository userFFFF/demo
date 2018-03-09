//
// Created by wu on 18-3-9.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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