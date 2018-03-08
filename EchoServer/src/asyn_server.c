//
// Created by wu on 18-3-8.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFERSIZE 256

void asyn_server(char *ip, short port)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        return;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if(ip == 0)
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        return;
    }

    if(listen(listenfd, 5) < 0) {
        return;
    }

    char buf[BUFFERSIZE];
    printf("server start: %s %d\n", inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, buf, sizeof(buf)), ntohs(servaddr.sin_port));

    int maxfd = listenfd;
    int maxi = -1;

    int client[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; ++i) {
        client[i] = -1;
    }

    fd_set allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for(;;) {
        fd_set rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset)) {
            struct sockaddr_in client_addr;
            socklen_t clilen = sizeof(client_addr);
            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &clilen);

            printf("client IP: %s %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, buf, sizeof(buf)), ntohs(client_addr.sin_port));

            int i;
            for (i = 0; i < FD_SETSIZE; ++i) {
                if(client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if(i == FD_SETSIZE) {
                printf("too many clients\n");
                close(connfd);
            } else {
                FD_SET(connfd, &allset);
                if(connfd > maxfd)
                    maxfd = connfd;
                if(i > maxi)
                    maxi = i;
            }

            if(--nready <= 0)
                continue;
        }

        for (int i = 0; i <= maxi; ++i) {
            int sockfd = client[i];
            if(sockfd < 0)
                continue;

            if(FD_ISSET(sockfd, &rset)) {
                ssize_t n;
                char buf[BUFFERSIZE];
                if((n = read(sockfd, buf, BUFFERSIZE)) == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[-1];
                } else {
                    write(sockfd, buf, n);
                }

                if(--nready <=0)
                    break;
            }
        }
    }
}
