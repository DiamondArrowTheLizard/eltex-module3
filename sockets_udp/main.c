#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define DEFAULT_IP "127.0.0.1"
#define EXIT_COMMAND "exit"

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);

    while (1) {
        ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, 
                             (struct sockaddr *)&remote_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            printf("\nMessage: %s\n> ", buffer);
            fflush(stdout);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <local_port> <remote_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int local_port = atoi(argv[1]);
    int remote_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(local_port);

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);
    remote_addr.sin_port = htons(remote_port);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock) != 0) {
        perror("pthread_create");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        printf("> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, EXIT_COMMAND) == 0) {
            break;
        }
        
        sendto(sock, buffer, strlen(buffer), 0, 
               (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    }

    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    
    close(sock);
    printf("Chat closed.\n");
    
    return 0;
}
