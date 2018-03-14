//
// Created by wu on 18-3-8.
//
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

#define MAXFD 1024

static void *do_echo(void *);

int listenfd = -1;
int client[MAXFD];

void sync_server(char *ip, short port)
{
    for(int i = 0; i < MAXFD; ++i)
        client[i] = -1;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        return;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if(ip == NULL || strlen(ip) ==0)
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        return;
    }

    if(listen(listenfd, 5) < 0) {
        return;
    }

    char buf[BUFSIZE];
    printf("server start: %s %d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, buf, sizeof(buf)), ntohs(servaddr.sin_port));

    for(;;) {
        struct sockaddr_in client_addr;
        socklen_t addr_len;
        int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);

        if(connfd < 0)
            break;

        int i;
        for(i = 0; i < MAXFD; ++i) {
            if(client[i] == -1) {
                client[i] = connfd;
                break;
            }
        }

        if(i == MAXFD) {
            printf("too many clients\n");
            close(connfd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, &do_echo, (void *) connfd);

        printf("client IP: %s %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, buf, sizeof(buf)), ntohs(client_addr.sin_port));
    }
}

static void *do_echo(void *arg)
{
    pthread_detach(pthread_self());
    int sockfd = (int) arg;
    ssize_t n;
    char buf[BUFSIZE];

    while((n = read(sockfd, buf, sizeof(buf))) > 0)
        write(sockfd, buf, n);

    for(int i = 0; i < MAXFD; ++i) {
        if(client[i] == sockfd) {
            client[i] = -1;
            break;
        }
    }
    close(sockfd);
}

void stop_sync_server()
{
    close(listenfd);
    for(int i = 0; i < MAXFD; ++i) {
        if(client[i] != -1) {
            close(client[i]);
        }
    }
}