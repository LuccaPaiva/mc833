#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>


#define LISTENQ 10
#define MAXDATASIZE 100

int get_available_port(struct sockaddr_in *addr) {
    for (int port = 1024; port <= 65535; port++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }

        // bzero(&addr, sizeof(addr)); // This line is no longer needed
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        addr->sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (struct sockaddr *)addr, sizeof(*addr)) == 0) {
            close(sockfd);
            return port;
        }

        close(sockfd);
    }

    return -1;
}

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];
    time_t ticks;

    int port = get_available_port(&servaddr);

    if (port == -1){
        printf("No available ports found\n");
        return 1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(1);
    }


    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server IP: %s\n", inet_ntoa(servaddr.sin_addr));
    printf("port: %d\n", port);

    for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
            perror("Erro ao aceitar a conexão");
            exit(1);
        }

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));

        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        if (getpeername(connfd, (struct sockaddr *)&cliaddr, &clilen) == -1) {
            perror("getpeername");
            exit(1);
        }

        printf("Client IP: %s\n", inet_ntoa(cliaddr.sin_addr));
        printf("Client Port: %d\n", ntohs(cliaddr.sin_port));

        write(connfd, buf, strlen(buf));

        // Recebe e imprime uma mensagem do cliente
        ssize_t n = read(connfd, buf, MAXDATASIZE);
        if (n < 0) {
            perror("Erro ao ler a mensagem do cliente");
            close(connfd);
            continue;
        } else if (n == 0) {
            // Conexão encerrada pelo cliente
            close(connfd);
            continue;
        } else {
            buf[n] = '\0'; // Adiciona um terminador de string
            printf("Mensagem do cliente: %s\n", buf);

            // Fecha a conexão após receber e imprimir a mensagem
            close(connfd);
            break; // Sai do loop para aceitar novas conexões
        }
    }
    return(0);
}
