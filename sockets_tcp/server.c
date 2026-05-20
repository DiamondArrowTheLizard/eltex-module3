#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>

#define FILE_NAME_SIZE 512
#define BUFFER_SIZE 2048
#define BACKLOG 5
#define PROTOCOL_AUTO 0
#define IPV4_ADDR_LEN 4

#define CMD_MATH "MATH"
#define CMD_GET "GET"
#define CMD_PUT "PUT"
#define CMD_QUIT "QUIT"

#define RESP_ERROR "ERROR\n"
#define RESP_READY "READY\n"
#define FMT_RESULT "RESULT %d\n"
#define FMT_FILE "FILE %ld\n"
#define FMT_SRV_FILE "srv_%s"

#define MODE_READ "rb"
#define MODE_WRITE "wb"

#define FMT_SCAN_CMD "%511s"
#define FMT_SCAN_MATH "%*s %c %d %d"
#define FMT_SCAN_GET "%*s %511s"
#define FMT_SCAN_PUT "%*s %511s %ld"

#define STR_WELCOME "TCP SERVER DEMO\n"
#define STR_ERR_PORT "ERROR, no port provided\n"
#define STR_ERR_SOCKET "ERROR opening socket"
#define STR_ERR_BIND "ERROR on binding"
#define STR_ERR_ACCEPT "ERROR on accept"
#define STR_ERR_FORK "ERROR on fork"
#define STR_CONN_INFO "+%s [%s] new connect!\n"
#define STR_UNKNOWN_HOST "Unknown host"
#define STR_DISCONNECT "-disconnect\n"
#define STR_USERS_ONLINE "%d user on-line\n"
#define STR_NO_USERS "No User on line\n"

volatile sig_atomic_t nclients = 0;
int sockfd = -1;

void printusers() {
    if (nclients) {
        printf(STR_USERS_ONLINE, nclients);
    } else {
        printf(STR_NO_USERS);
    }
}

void handle_sigint(int sig) {
    (void)sig;
    if (sockfd >= 0) {
        close(sockfd);
    }
    exit(EXIT_SUCCESS);
}

void handle_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        nclients--;
    }
}

int myfunc(char op, int a, int b) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/') return (b != 0) ? a / b : 0;
    return 0;
}

void dostuff(int sock) {
    char buff[BUFFER_SIZE];
    char cmd[FILE_NAME_SIZE];
    int n;

    while ((n = recv(sock, buff, BUFFER_SIZE - 1, 0)) > 0) {
        buff[n] = '\0';
        if (sscanf(buff, FMT_SCAN_CMD, cmd) <= 0) {
            continue;
        }

        if (strcmp(cmd, CMD_MATH) == 0) {
            char op;
            int a, b;
            sscanf(buff, FMT_SCAN_MATH, &op, &a, &b);
            int res = myfunc(op, a, b);
            
            char resp[BUFFER_SIZE];
            snprintf(resp, sizeof(resp), FMT_RESULT, res);
            send(sock, resp, strlen(resp), 0);
        } 
        else if (strcmp(cmd, CMD_GET) == 0) {
            char filename[FILE_NAME_SIZE];
            sscanf(buff, FMT_SCAN_GET, filename);
            FILE *f = fopen(filename, MODE_READ);
            if (!f) {
                send(sock, RESP_ERROR, strlen(RESP_ERROR), 0);
            } else {
                fseek(f, 0, SEEK_END);
                long size = ftell(f);
                fseek(f, 0, SEEK_SET);
                
                char resp[BUFFER_SIZE];
                snprintf(resp, sizeof(resp), FMT_FILE, size);
                send(sock, resp, strlen(resp), 0);
                
                long total_sent = 0;
                while (total_sent < size) {
                    long remaining = size - total_sent;
                    int to_read = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
                    int r = fread(buff, 1, to_read, f);
                    send(sock, buff, r, 0);
                    total_sent += r;
                }
                fclose(f);
            }
        } 
        else if (strcmp(cmd, CMD_PUT) == 0) {
            char filename[FILE_NAME_SIZE];
            long size = 0;
            sscanf(buff, FMT_SCAN_PUT, filename, &size);
            
            char out_filename[BUFFER_SIZE];
            snprintf(out_filename, sizeof(out_filename), FMT_SRV_FILE, filename);
            FILE *f = fopen(out_filename, MODE_WRITE);
            if (!f) {
                send(sock, RESP_ERROR, strlen(RESP_ERROR), 0);
            } else {
                send(sock, RESP_READY, strlen(RESP_READY), 0);
                long total_rcvd = 0;
                while (total_rcvd < size) {
                    long remaining = size - total_rcvd;
                    int to_recv = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
                    int r = recv(sock, buff, to_recv, 0);
                    if (r <= 0) {
                        break;
                    }
                    fwrite(buff, 1, r, f);
                    total_rcvd += r;
                }
                fclose(f);
            }
        }
        else if (strcmp(cmd, CMD_QUIT) == 0) {
            break;
        }
    }
    printf(STR_DISCONNECT);
}

int main(int argc, char *argv[]) {
    int newsockfd;
    int portno;
    int pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, handle_sigchld);

    printf(STR_WELCOME);

    if (argc < 2) {
        fprintf(stderr, STR_ERR_PORT);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, PROTOCOL_AUTO);
    if (sockfd < 0) {
        perror(STR_ERR_SOCKET);
        exit(EXIT_FAILURE);
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror(STR_ERR_BIND);
        exit(EXIT_FAILURE);
    }

    listen(sockfd, BACKLOG);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror(STR_ERR_ACCEPT);
            continue;
        }
        nclients++;

        struct hostent *hst;
        hst = gethostbyaddr((char *)&cli_addr.sin_addr, IPV4_ADDR_LEN, AF_INET);
        printf(STR_CONN_INFO,
               (hst) ? hst->h_name : STR_UNKNOWN_HOST,
               (char*)inet_ntoa(cli_addr.sin_addr));
        printusers();

        pid = fork();
        if (pid < 0) {
            perror(STR_ERR_FORK);
        }
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            close(newsockfd);
            exit(EXIT_SUCCESS);
        } else {
            close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}
