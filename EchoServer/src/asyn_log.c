//
// Created by wu on 18-3-9.
//
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#define QUEUESIZE 4096

typedef struct _QUEUE
{
    char *log[QUEUESIZE];
    int head;
    int tail;
}T_QUEUE, *PT_QUEUE;

int IsQueueEmpty(PT_QUEUE ptQueue)
{
    return ptQueue->head == ptQueue->tail;
}

int IsQueueFull(PT_QUEUE ptQueue)
{
    return (ptQueue->tail + 1) % QUEUESIZE == ptQueue->head;
}

char logFile[PATH_MAX];
int logFileSize = 10;
int logFileNum = 10;

T_QUEUE logQueue;
pthread_mutex_t mutex;

static void *do_writelog(void *);

void logInit(char *dir, char *filename, int logMSize, int logNum)
{
    strcpy(logFile, dir);
    strcat(logFile, "/");
    strcat(logFile, filename);

    logFileSize = logMSize;
    logFileNum = logNum;

    logQueue.head = 0;
    logQueue.tail = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_t tid;
    pthread_create(&tid, NULL, &do_writelog, NULL);
}

void log(char *str)
{
    if(IsQueueFull(&logQueue)) {
        printf("Queue is full...\n");
        return;
    }

    int len = strlen(str);
    char *log = malloc(len+1);
    strcpy(log, str);

    pthread_mutex_lock(&mutex);
    logQueue.log[logQueue.tail]= log;
    logQueue.tail = (logQueue.tail + 1) % QUEUESIZE;
    pthread_mutex_unlock(&mutex);
}

void renameLogfiles()
{
    char lastLogFilename[PATH_MAX];
    sprintf(lastLogFilename, "%s_%d", logFile, logFileNum);
    remove(lastLogFilename);

    for (int i = logFileNum - 1; i > 0 ; --i) {
        char oldFilename[PATH_MAX];
        char newFilename[PATH_MAX];
        sprintf(oldFilename, "%s_%d", logFile, i);
        sprintf(newFilename, "%s_%d", logFile, i+1);

        rename(oldFilename, newFilename);
    }

    char firstFilename[PATH_MAX];
    sprintf(firstFilename, "%s_%d", logFile, 1);
    rename(logFile, firstFilename);
}

void writeToFile(char *str)
{
    struct stat statbuff;
    if(access(logFile, F_OK) == 0 && stat(logFile, &statbuff) == 0) {
        int filesize = statbuff.st_size;

        if(filesize >= logFileSize * 1024 *1024) {
            renameLogfiles();
        }
    }

    time_t tm;
    time(&tm);
    struct tm* tm_t = localtime(&tm);
    char tmBuf[BUFSIZ];
    sprintf(tmBuf, "%04d-%02d-%02d %02d:%02d:%02d: ", 1900+tm_t->tm_year, tm_t->tm_mon, tm_t->tm_mday, tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec);

    FILE *fp = fopen(logFile, "a+");
    fwrite(tmBuf, strlen(tmBuf), 1, fp);
    fwrite(str, strlen(str), 1, fp);
    fwrite("\n", 1, 1, fp);
    fclose(fp);
}

static void *do_writelog(void *arg)
{
    while(1) {
        if(!IsQueueEmpty(&logQueue)) {
            pthread_mutex_lock(&mutex);
            char *log = logQueue.log[logQueue.head];
            logQueue.head = (logQueue.head + 1) % QUEUESIZE;
            pthread_mutex_unlock(&mutex);

            writeToFile(log);
        } else
            usleep(100);
    }
}