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
#include <sys/wait.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096
#define NUMCONNECTIONS 2

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

int is_socket_open(int sockfd) {
    int optval;
    socklen_t optlen = sizeof(optval);

    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1) {
        perror("getsockopt");
        return 0; // Erro ao obter as opções do soquete
    }

    if (optval == 0)
        return 1; // O soquete está aberto
    else
        return 0; // O soquete não está aberto
}

int send_command(int connfd) {
    // Gere um número aleatório entre 1 e 3
    char buf[MAXLINE];
    int numero = rand() % 3;
    
    // Crie um array de strings com três opções
    char *str[3] = {"Opção A", "Opção B", "Stop"};
    
    snprintf(buf, sizeof(buf), "%s", str[numero]);
    write(connfd, buf, strlen(buf));
    return numero;
}

void write_log(FILE *log_file, const char* str){
    printf("File: %s", str);
    fprintf(log_file, "%s", str);
}

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main (int argc, char **argv) {
    int    listenfd, connfd, numConections = NUMCONNECTIONS;
    struct sockaddr_in servaddr;
    char   error[MAXLINE + 1], line[MAXLINE + 1], recvline[MAXLINE + 1];
    time_t ticks;

    if (argc != 2) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <Port>");
        perror(error);
        exit(1);
    }

    FILE *log_file = fopen("server_log.txt", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }

    // Configura o tratamento de sinais SIGCHLD para evitar zumbis
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        fclose(log_file);
        exit(1);
    }

    // Obtém uma porta disponível usando a função get_available_port
    int port = atoi(argv[1]);
    if (port == -1){
        printf("No available ports found\n");
        fclose(log_file);
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

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

    while (numConections) {
        // Aceita uma conexão de cliente
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
            perror("Erro ao aceitar a conexão");
            fclose(log_file);
            exit(1);
        }
        numConections--;
        // Cria um novo processo filho para lidar com o cliente
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            fclose(log_file);
            exit(1);
        }

        if (child_pid == 0) { // Processo filho
            //close(listenfd); // Fecha o socket de escuta no processo filho

            // Obtém o tempo atual e prepara uma mensagem de resposta
            int num = 0;
            ticks = time(NULL);
            snprintf(line, sizeof(line), "Connection at time: %.24s\r\n", ctime(&ticks));
            write_log(log_file, line);

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

            snprintf(line, sizeof(line), "Server Tread: %d\n", getpid());
            write_log(log_file, line);

            int n = 1;
            while(num != 2 && n){
                // Chama a função para enviar um comando ao cliente
                num = send_command(connfd);
                n = read(connfd, recvline, MAXLINE);
            }
            
            exit(0); // O processo filho deve sair quando terminar o atendimento ao cliente
        }
        close(connfd); // Fecha o socket no processo pai, pois o processo filho lidará com o cliente
    }

    fclose(log_file);
    return(0);
}
