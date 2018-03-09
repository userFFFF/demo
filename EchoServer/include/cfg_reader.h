//
// Created by wu on 18-3-9.
//

#ifndef ECHOSERVER_CFG_READER_H
#define ECHOSERVER_CFG_READER_H

typedef struct _KVPair
{
    char *key;
    char *value;
    struct _KVPair *next;
}KVPair;

KVPair *readFromFile(char *filename);

char *getValue(KVPair *kv, char *key);

void freeKVPair(KVPair *kv)

#endif //ECHOSERVER_CFG_READER_H
