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
#define NUMCONNECTIONS 10

void write_log(FILE *log_file, const char* str){
    fprintf(log_file, "%s", str);
    fflush(log_file);
}

int Socket(int family, int type, int flags, FILE *log_file) {
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) {
        perror("socket");
        fclose(log_file);
        exit(1);
    } else
        return sockfd;
}

int Bind(int listenfd, struct sockaddr *servaddr, socklen_t addrlen, FILE *log_file) {
    char line[MAXLINE + 1]; // Declare a variável line aqui

    if (bind(listenfd, servaddr, addrlen) == -1) {
        perror("bind");
        fclose(log_file);
        exit(1); // Encerra o programa em caso de erro no bind
    } else {
        // Exibe o endereço IP do servidor e a porta
        snprintf(line, sizeof(line), "Server IP: %s\n", inet_ntoa(((struct sockaddr_in *)servaddr)->sin_addr));
        write_log(log_file, line);
        snprintf(line, sizeof(line), "Server port: %d\n", ntohs(((struct sockaddr_in *)servaddr)->sin_port));
        write_log(log_file, line);
        return 0; // Retorna 0 para indicar sucesso
    }
}



void Listen(int listenfd, int backlog, FILE *log_file) {
    if (listen(listenfd, backlog) == -1) {
        perror("listen");
        fclose(log_file);
        exit(1);
    }
}

int Accept(int listenfd, FILE *log_file){
    int connfd;
    if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
        perror("Erro ao aceitar a conexão");
        fclose(log_file);
        exit(1);
    }
    return connfd;
}

pid_t Fork(FILE *log_file){
    pid_t child_pid;
    if ( (child_pid = fork()) == -1) {
        perror("fork");
        fclose(log_file);
        exit(1);
    }
    return child_pid;
}

void GetPeerName(int sockfd, struct sockaddr *addr, socklen_t *addrlen, FILE *log_file) {
    if (getpeername(sockfd, addr, addrlen) == -1) {
        perror("getpeername");
        fclose(log_file);
        exit(1);
    }
}

//Funcao que 
int send_command(int connfd, int id, FILE* log_file) {
    char buf[MAXLINE];
    // Gere um número aleatório entre 1 e 3
    int numero = rand() % 3;
    
    // Crie um array de strings com três opções
    char *str[3] = {"SIMULE: CPU_INTENSIVA", "SIMULE: MEMORIA_INTENSIVA", "DESCONECTE"};
    
    snprintf(buf, sizeof(buf), "%s", str[numero]);
    write(connfd, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "%d - %s\n", id, str[numero]);
    write_log(log_file, buf);
    return numero;
}

void HandleChildProcess(int child_pid, int listenfd, int connfd, FILE *log_file, char *recvline) {
    if (child_pid == 0) { // Processo filho
        close(listenfd); // Fecha o socket de escuta no processo filho

        // Obtém o tempo atual e prepara uma mensagem de resposta
        char line[MAXLINE + 1];

        // Obtém informações sobre o cliente (endereço IP e porta)
        struct sockaddr_in cliaddr;

        printf("Conexao Aceita de %s\n", inet_ntoa(cliaddr.sin_addr));

        exit(0); // O processo filho deve sair quando terminar o atendimento ao cliente
    }
    close(connfd); // Fecha o socket no processo pai, pois o processo filho lidará com o cliente
}



int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   error[MAXLINE + 1], recvline[MAXLINE + 1];

    if (argc != 3) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <Port>");
        perror(error);
        strcat(error, " <backlog>");
        perror(error);
        exit(1);
    }

    FILE *log_file = fopen("server_log.txt", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }

    // Obtém uma porta disponível usando a função get_available_port
    int port = atoi(argv[1]);
    int backlog = atoi(argv[2]);
    
    if (port == -1){
        printf("No available ports found\n");
        fclose(log_file);
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Cria um socket para ouvir conexões
    listenfd = Socket(AF_INET, SOCK_STREAM, 0, log_file);
    // Associa o socket à porta e endereço IP obtidos
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr), log_file);

    int udpfd = Socket(AF_INET, SOCK_DGRAM, 0, log_file);
    Bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr), log_file);

    Listen(listenfd, backlog, log_file);

    fd_set all_fds, read_fds;
    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds); // Adiciona o socket TCP ao conjunto
    FD_SET(udpfd, &all_fds);    // Adiciona o socket UDP ao conjunto

    time_t start_time = time(NULL);

    printf("Server IP: %s\n", inet_ntoa((servaddr.sin_addr)));
    sleep(5);
    int numConections = NUMCONNECTIONS;
    while (1) {
        read_fds = all_fds; // Copia o conjunto para preservar o original

        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            fclose(log_file);
            exit(1);
        }

        // Aceita uma conexão de cliente
        connfd = Accept(listenfd, log_file);
        // Cria um novo processo filho para lidar com o cliente

        // Verifica se há atividade no socket TCP
        if (FD_ISSET(listenfd, &read_fds)) {
            printf("TCP\n");
            pid_t child_pid = Fork(log_file);
            HandleChildProcess(child_pid, listenfd, connfd, log_file, recvline);
        }

        // Verifica se há atividade no socket UDP
        if (FD_ISSET(udpfd, &read_fds)) {
            printf("UDP\n");
            pid_t child_pid = Fork(log_file);
            HandleChildProcess(child_pid, udpfd, connfd, log_file, recvline);
        }
    }


    fclose(log_file);
    return(0);
}
