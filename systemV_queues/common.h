#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define SERVER_TYPE 10
#define MAX_TEXT 256
#define MAX_CLIENTS 50

#define CHAT_KEY "sysv_queue"


struct msg_buf {
    long mtype;
    int sender_id;
    char mtext[MAX_TEXT];
};
