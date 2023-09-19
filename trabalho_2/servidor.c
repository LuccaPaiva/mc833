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
#include <pthread.h> // Biblioteca para threads

#define LISTENQ 10
#define MAXDATASIZE 100

// Estrutura para passar dados para a função da thread
struct ThreadData {
    int connfd;
    struct sockaddr_in cliaddr;
};

// Função para encontrar uma porta disponível
int get_available_port(struct sockaddr_in *addr) {
    for (int port = 1024; port <= 65535; port++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }

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

// Função executada por cada thread
void *client_handler(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int connfd = data->connfd;
    struct sockaddr_in cliaddr = data->cliaddr;
    char buf[MAXDATASIZE];
    time_t ticks;

    ticks = time(NULL);
    snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));

    // Envia a mensagem de resposta para o cliente
    write(connfd, buf, strlen(buf));

    printf("Client IP: %s\n", inet_ntoa(cliaddr.sin_addr));
    printf("Client Port: %d\n", ntohs(cliaddr.sin_port));

    close(connfd);
    free(arg); // Libera a estrutura de dados alocada dinamicamente
    return NULL;
}

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr, addr;
    char   error[MAXDATASIZE + 1];

    int port = get_available_port(&addr);

    if (port == -1){
        printf("No available ports found\n");
        return 1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(1);
    }

    servaddr = addr;

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    printf("IP: %s\n", inet_ntoa(servaddr.sin_addr));
    printf("port: %d\n", port);

    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) == -1 ) {
            perror("accept");
            exit(1);
        }

        // Aloca espaço para os dados que serão passados para a thread
        struct ThreadData *data = (struct ThreadData *)malloc(sizeof(struct ThreadData));
        data->connfd = connfd;
        data->cliaddr = cliaddr;

        // Cria uma nova thread para lidar com o cliente
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, data) != 0) {
            perror("pthread_create");
            free(data);
        }
    }

    close(listenfd);
    return(0);
}
