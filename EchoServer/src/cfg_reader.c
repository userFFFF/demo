//
// Created by wu on 18-3-9.
//

#include "cfg_reader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

int getPos(char *str, char c)
{
    int pos = -1;
    int len = strlen(str);
    for (int i = 0; i < len; ++i) {
        if(str[i] == c) {
            pos = i;
            break;
        }
    }
    return pos;
}
KVPair *readFromFile(char *filename)
{
    FILE *fp = fopen(filename,"r");
    if(fp== NULL)
        return NULL;

    KVPair *head = NULL;
    KVPair *tmp = NULL;

    char buf[BUFSIZE] = {0};
    while(fgets(buf, sizeof(buf), fp) != NULL) {
        if(buf[0] == '#')
            continue;

        int len = strlen(buf);
        int pos = getPos(buf, '=');
        if(pos == -1)
            continue;

        char *key = malloc(pos+1);
        memset(key, 0, pos+1);
        memcpy(key, buf, pos);
        char *value = malloc(len-pos-1);
        memset(value, 0, len-pos-1);
        memcpy(value, buf+pos+1, len-pos-2);

        KVPair *kv = malloc(sizeof(KVPair));
        kv->key = key;
        kv->value = value;
        kv->next = NULL;

        if(head == NULL) {
            head = kv;
        } else {
            tmp->next = kv;
        }
        tmp = kv;
    }

    return head;
}

char *getValue(KVPair *kv, char *key)
{
    KVPair *tmp = kv;
    do
    {
        if(strcmp(key, tmp->key) == 0) {
            return tmp->value;
        }
    }while (tmp = tmp->next);

    return  NULL;
}

void freeKVPair(KVPair *kv)
{
    KVPair *tmp = kv;
    do
    {
        free(tmp->key);
        free(tmp->value);
    }while (tmp = tmp->next);
}
