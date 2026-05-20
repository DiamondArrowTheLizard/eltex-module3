#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define FILE_NAME_SIZE 512
#define BUFFER_SIZE 2048
#define PROTOCOL_AUTO 0

#define CMD_MATH "MATH"
#define CMD_GET "GET"
#define CMD_PUT "PUT"
#define CMD_QUIT "QUIT"

#define FMT_REQ_MATH "MATH %c %d %d\n"
#define FMT_REQ_GET "GET %s\n"
#define FMT_REQ_PUT "PUT %s %ld\n"
#define FMT_REQ_QUIT "QUIT\n"

#define MODE_READ "rb"
#define MODE_WRITE "wb"
#define FMT_CLI_FILE "cli_%s"

#define STR_MENU_OP "Choose operation (+, -, *, /): "
#define STR_MENU_NUM1 "Enter first number: "
#define STR_MENU_NUM2 "Enter second number: "
#define STR_MENU_FILE "Enter filename: "

#define FMT_RESULT_PRINT "Server Result: %d\n"
#define STR_ERR_FILE "File open error\n"
#define STR_DOWNLOAD_OK "Download completed successfully\n"
#define STR_UPLOAD_OK "Upload completed successfully\n"
#define STR_SERVER_ERR "Server returned error\n"
#define STR_INVALID_CHOICE "Invalid choice\n"

#define FMT_USAGE "usage %s hostname port\n"
#define STR_ERR_SOCKET "ERROR opening socket"
#define STR_ERR_HOST "ERROR, no such host\n"
#define STR_ERR_CONNECT "ERROR connecting"

#define FMT_MENU "1. Math Operations\n2. Download File\n3. Upload File\n4. Exit\nChoice: "
#define FMT_SCAN_CHOICE "%d"
#define FMT_SCAN_OP " %c"
#define FMT_SCAN_NUM "%d"
#define FMT_SCAN_STR "%511s"
#define STR_CLIENT_DEMO "TCP DEMO CLIENT\n"

#define FMT_SCAN_RESULT "RESULT %d"
#define FMT_SCAN_FILE "FILE %ld"

int my_sock = -1;

void handle_sigint(int sig) {
    (void)sig;
    if (my_sock >= 0) {
        send(my_sock, FMT_REQ_QUIT, strlen(FMT_REQ_QUIT), 0);
        close(my_sock);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    signal(SIGINT, handle_sigint);

    printf(STR_CLIENT_DEMO);
    if (argc < 3) {
        fprintf(stderr, FMT_USAGE, argv[0]);
        exit(EXIT_FAILURE);
    }

    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, PROTOCOL_AUTO);
    if (my_sock < 0) {
        perror(STR_ERR_SOCKET);
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, STR_ERR_HOST);
        exit(EXIT_FAILURE);
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror(STR_ERR_CONNECT);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf(FMT_MENU);
        int choice;
        if (scanf(FMT_SCAN_CHOICE, &choice) <= 0) {
            break;
        }

        if (choice == 1) {
            char op;
            int a, b;
            printf(STR_MENU_OP);
            scanf(FMT_SCAN_OP, &op);
            printf(STR_MENU_NUM1);
            scanf(FMT_SCAN_NUM, &a);
            printf(STR_MENU_NUM2);
            scanf(FMT_SCAN_NUM, &b);

            char req[BUFFER_SIZE];
            snprintf(req, sizeof(req), FMT_REQ_MATH, op, a, b);
            send(my_sock, req, strlen(req), 0);

            char resp[BUFFER_SIZE];
            int n = recv(my_sock, resp, BUFFER_SIZE - 1, 0);
            if (n <= 0) {
                break;
            }
            resp[n] = '\0';

            int res;
            sscanf(resp, FMT_SCAN_RESULT, &res);
            printf(FMT_RESULT_PRINT, res);
        }
        else if (choice == 2) {
            char filename[FILE_NAME_SIZE];
            printf(STR_MENU_FILE);
            scanf(FMT_SCAN_STR, filename);

            char req[BUFFER_SIZE];
            snprintf(req, sizeof(req), FMT_REQ_GET, filename);
            send(my_sock, req, strlen(req), 0);

            char resp[BUFFER_SIZE];
            int n = recv(my_sock, resp, BUFFER_SIZE - 1, 0);
            if (n <= 0) {
                break;
            }
            resp[n] = '\0';

            if (strncmp(resp, "ERROR", 5) == 0) {
                printf(STR_SERVER_ERR);
            } else {
                long size = 0;
                sscanf(resp, FMT_SCAN_FILE, &size);

                char out_name[BUFFER_SIZE];
                snprintf(out_name, sizeof(out_name), FMT_CLI_FILE, filename);
                FILE *f = fopen(out_name, MODE_WRITE);
                if (!f) {
                    printf(STR_ERR_FILE);
                    continue;
                }

                long total_rcvd = 0;
                while (total_rcvd < size) {
                    long remaining = size - total_rcvd;
                    int to_recv = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
                    int r = recv(my_sock, resp, to_recv, 0);
                    if (r <= 0) {
                        break;
                    }
                    fwrite(resp, 1, r, f);
                    total_rcvd += r;
                }
                fclose(f);
                printf(STR_DOWNLOAD_OK);
            }
        }
        else if (choice == 3) {
            char filename[FILE_NAME_SIZE];
            printf(STR_MENU_FILE);
            scanf(FMT_SCAN_STR, filename);

            FILE *f = fopen(filename, MODE_READ);
            if (!f) {
                printf(STR_ERR_FILE);
                continue;
            }

            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);

            char req[BUFFER_SIZE];
            snprintf(req, sizeof(req), FMT_REQ_PUT, filename, size);
            send(my_sock, req, strlen(req), 0);

            char resp[BUFFER_SIZE];
            int n = recv(my_sock, resp, BUFFER_SIZE - 1, 0);
            if (n <= 0) {
                fclose(f);
                break;
            }
            resp[n] = '\0';

            if (strncmp(resp, "READY", 5) == 0) {
                long total_sent = 0;
                while (total_sent < size) {
                    long remaining = size - total_sent;
                    int to_read = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (int)remaining;
                    int r = fread(resp, 1, to_read, f);
                    send(my_sock, resp, r, 0);
                    total_sent += r;
                }
                printf(STR_UPLOAD_OK);
            } else {
                printf(STR_SERVER_ERR);
            }
            fclose(f);
        }
        else if (choice == 4) {
            send(my_sock, FMT_REQ_QUIT, strlen(FMT_REQ_QUIT), 0);
            break;
        }
        else {
            printf(STR_INVALID_CHOICE);
        }
    }

    close(my_sock);
    return 0;
}
