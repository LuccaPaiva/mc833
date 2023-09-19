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
#define MAXLINE 4096


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

    return -1; // Retorna -1 se nenhuma porta estiver disponível
}

void send_command(int connfd) {
    // Gere um número aleatório entre 1 e 3
    int numero = rand() % 3 + 1;
    
    // Crie um array de strings com três opções
    char *str[3] = {"Opção A", "Opção B", "Opção C"};
    
    // Envie a opção escolhida para o cliente
    write(connfd, str[numero - 1], strlen(str[numero - 1]));
}

void write_log(FILE *log_file, const char* str){
    printf("File: %s", str);
    fprintf(log_file, "%s", str);
}

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];
    char   error[MAXLINE + 1], line[MAXLINE + 1];
    time_t ticks;

    if (argc != 2) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <IPaddress>");
        perror(error);
        exit(1);
    }

    FILE *log_file = fopen("server_log.txt", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }

    // Obtém uma porta disponível usando a função get_available_port
    //int port = get_available_port(&servaddr);
    int port = atoi(argv[1]);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;


    if (port == -1){
        printf("No available ports found\n");
        return 1;
    }

    // Cria um socket para ouvir conexões
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        fclose(log_file);
        exit(1);
    }

    // Associa o socket à porta e endereço IP obtidos
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        fclose(log_file);
        exit(1);
    }

    // Inicia a escuta por conexões
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        fclose(log_file);
        exit(1);
    }

    // Exibe o endereço IP do servidor e a porta
    snprintf(line, sizeof(line), "Server IP: %s\n", inet_ntoa(servaddr.sin_addr));
    write_log(log_file, line);
    snprintf(line, sizeof(line), "Server port: %d\n", ntohs(servaddr.sin_port));
    write_log(log_file, line);


    for (;;) {
        // Aceita uma conexão de cliente
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
            perror("Erro ao aceitar a conexão");
            fclose(log_file);
            exit(1);
        }

        // Obtém o tempo atual e prepara uma mensagem de resposta
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));

        // Obtém informações sobre o cliente (endereço IP e porta)
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        if (getpeername(connfd, (struct sockaddr *)&cliaddr, &clilen) == -1) {
            perror("getpeername");
            fclose(log_file);
            exit(1);
        }

        snprintf(line, sizeof(line), "Client connected from port: %d\n", ntohs(cliaddr.sin_port));
        write_log(log_file, line);

        // Envia a mensagem de resposta para o cliente
        write(connfd, buf, strlen(buf));

        send_command(connfd);

        close(connfd);
    }
    fclose(log_file);
    return(0);
}
