#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int global_msqid;

void log_event(const char* event, int client_id, const char* text) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    
    if (text) {
        printf("Client %d %s: \"%s\"\n", client_id, event, text);
    } else {
        printf("Client %d %s\n", client_id, event);
    }
}

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SERVER] Shutting down and removing queue...\n");
    msgctl(global_msqid, IPC_RMID, NULL);
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);
    key_t key = ftok(CHAT_KEY, 65);
    int msqid = msgget(key, 0666 | IPC_CREAT);
    global_msqid = msqid;

    struct msg_buf msg;
    int clients[MAX_CLIENTS];
    int client_count = 0;

    printf("[SERVER] Started. Waiting for connections...\n");

    while (1) {
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SERVER_TYPE, 0) == -1) break;

        if (strcmp(msg.mtext, "CONNECT") == 0) {
            int exists = 0;
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == msg.sender_id) exists = 1;
            }
            if (!exists && client_count < MAX_CLIENTS) {
                clients[client_count++] = msg.sender_id;
                log_event("registered and online", msg.sender_id, NULL);
            }
            continue;
        }

        if (strcmp(msg.mtext, "shutdown") == 0) {
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == msg.sender_id) {
                    clients[i] = clients[--client_count];
                    log_event("disconnected (shutdown)", msg.sender_id, NULL);
                    break;
                }
            }
            continue;
        }

        log_event("sent message", msg.sender_id, msg.mtext);
        int sender = msg.sender_id;
        for (int i = 0; i < client_count; i++) {
            if (clients[i] != sender) {
                msg.mtype = clients[i];
                msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0);
            }
        }
    }
    return 0;
}
