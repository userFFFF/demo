//
// Created by wu on 18-3-12.
//

#ifndef ECHOSERVER_ASYN_LOG_H
#define ECHOSERVER_ASYN_LOG_H

void logInit(char *dir, char *filename, int logMSize, int logNum);

void asyn_log(char *str);

#endif //ECHOSERVER_ASYN_LOG_H
