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
#include <sys/select.h>
#include <errno.h>

#define FILE_NAME_SIZE 512
#define BUFFER_SIZE 2048
#define BACKLOG 5
#define PROTOCOL_AUTO 0
#define IPV4_ADDR_LEN 4
#define MAX_CLIENTS 32
#define VAL_NOT_SET -1

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

#define STR_WELCOME "TCP MULTIPLEXED SERVER DEMO\n"
#define STR_ERR_PORT "ERROR, no port provided\n"
#define STR_ERR_SOCKET "ERROR opening socket"
#define STR_ERR_BIND "ERROR on binding"
#define STR_ERR_ACCEPT "ERROR on accept"
#define STR_CONN_INFO "+%s [%s] new connect!\n"
#define STR_UNKNOWN_HOST "Unknown host"
#define STR_DISCONNECT "-disconnect\n"
#define STR_USERS_ONLINE "%d user(s) on-line\n"
#define STR_NO_USERS "No User on line\n"
#define STR_SELECT_ERR "Select error"

volatile sig_atomic_t running = 1;
int sockfd = VAL_NOT_SET;
int client_sockets[MAX_CLIENTS];
int nclients = 0;

void printusers() {
    if (nclients) {
        printf(STR_USERS_ONLINE, nclients);
    } else {
        printf(STR_NO_USERS);
    }
}

void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

int myfunc(char op, int a, int b) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/') return (b != 0) ? a / b : 0;
    return 0;
}

void handle_client_request(int slot) {
    int sock = client_sockets[slot];
    char buff[BUFFER_SIZE];
    char cmd[FILE_NAME_SIZE];
    int n = recv(sock, buff, BUFFER_SIZE - 1, 0);

    if (n <= 0) {
        printf(STR_DISCONNECT);
        close(sock);
        client_sockets[slot] = VAL_NOT_SET;
        nclients--;
        printusers();
        return;
    }

    buff[n] = '\0';
    if (sscanf(buff, FMT_SCAN_CMD, cmd) <= 0) {
        return;
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
        printf(STR_DISCONNECT);
        close(sock);
        client_sockets[slot] = VAL_NOT_SET;
        nclients--;
        printusers();
    }
}

int main(int argc, char *argv[]) {
    int newsockfd;
    int portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf(STR_WELCOME);

    if (argc < 2) {
        fprintf(stderr, STR_ERR_PORT);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = VAL_NOT_SET;
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

    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int max_fd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > VAL_NOT_SET) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_fd) {
                max_fd = sd;
            }
        }

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror(STR_SELECT_ERR);
        }

        if (!running) {
            break;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd >= 0) {
                struct hostent *hst;
                hst = gethostbyaddr((char *)&cli_addr.sin_addr, IPV4_ADDR_LEN, AF_INET);
                printf(STR_CONN_INFO,
                       (hst) ? hst->h_name : STR_UNKNOWN_HOST,
                       (char*)inet_ntoa(cli_addr.sin_addr));

                int saved = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_sockets[i] == VAL_NOT_SET) {
                        client_sockets[i] = newsockfd;
                        nclients++;
                        saved = 1;
                        break;
                    }
                }
                if (!saved) {
                    close(newsockfd);
                }
                printusers();
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if ((sd > VAL_NOT_SET) && FD_ISSET(sd, &readfds)) {
                handle_client_request(i);
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > VAL_NOT_SET) {
            close(client_sockets[i]);
        }
    }
    close(sockfd);
    return 0;
}
