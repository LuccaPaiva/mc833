#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int execute_command(const char *str){
    sleep(1);
    printf("-%s executada com sucesso\n", str);
    return strcmp(str, "DESCONECTE");
}

int main(int argc, char **argv) {
    int    sockfd, n, servport, flag = 1;
    char   recvline[MAXLINE + 1], message[MAXLINE + 1];
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr, localaddr;
    socklen_t addrlen = sizeof(localaddr);

    // Verifica se o programa foi chamado com a quantidade correta de argumentos
    if (argc != 3) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <IPaddress>");
        perror(error, argv[1]);
        strcat(error, " <Port>");
        perror(error);
        exit(1);
    }

    // Cria um socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    // Configura o endereço do servidor para o qual vamos nos conectar
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servport = atoi(argv[2]);
    servaddr.sin_port = htons(servport);

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    // Estabelece uma conexão com o servidor
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    // Obtém informações sobre o socket local
    if (getsockname(sockfd, (struct sockaddr *)&localaddr, &addrlen) == -1) {
        perror("getsockname");
        exit(1);
    }

    // Exibe o endereço IP e o número da porta local
    printf("Local IP address: %s\n", inet_ntoa(localaddr.sin_addr));
    printf("Local port number: %d\n", ntohs(localaddr.sin_port));
    printf("Server IP address: %s\n", inet_ntoa(servaddr.sin_addr));
    printf("Server port number: %d\n", ntohs(servaddr.sin_port));


    // Aguarda e lê a resposta do servidor
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        flag = execute_command(recvline);
        snprintf(message, sizeof(message), "%d", flag);
        n = write(sockfd, message, strlen(message));
        if(!flag)
            break;
    }

    if(!flag){
        printf("Fechando coneção com servidor\n");
        close(sockfd);
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}
