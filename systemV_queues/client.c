#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    int my_id = atoi(argv[1]);
    key_t key = ftok(CHAT_KEY, 65);
    int msqid = msgget(key, 0666);
    struct msg_buf msg;

    msg.mtype = SERVER_TYPE;
    msg.sender_id = my_id;
    strcpy(msg.mtext, "CONNECT");
    msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0);

    pid_t pid = fork();
    if (pid == 0) {
        while (1) {
            if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), my_id, 0) == -1) break;
            printf("\nClient %d: %s\n> ", msg.sender_id, msg.mtext);
            fflush(stdout);
        }
    } else {
        while (1) {
            printf("> ");
            fgets(msg.mtext, MAX_TEXT, stdin);
            msg.mtext[strcspn(msg.mtext, "\n")] = 0;
            
            msg.mtype = SERVER_TYPE;
            msg.sender_id = my_id;
            msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0);

            if (strcmp(msg.mtext, "shutdown") == 0) {
                kill(pid, SIGKILL);
                break;
            }
        }
    }
    return 0;
}
