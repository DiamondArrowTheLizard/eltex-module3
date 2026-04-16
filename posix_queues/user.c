#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "params.h"

#define QUEUE_NAME_MAX_SIZE 100

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments (your name + receiver name).\n");
        exit(EXIT_FAILURE);
    }

    char my_queue[QUEUE_NAME_MAX_SIZE];
    char other_queue[QUEUE_NAME_MAX_SIZE];
    snprintf(my_queue, QUEUE_NAME_MAX_SIZE, "/%s", argv[1]);
    snprintf(other_queue, QUEUE_NAME_MAX_SIZE, "/%s", argv[2]);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MESSAGE_MAX_COUNT;
    attr.mq_msgsize = MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq_recv = mq_open(my_queue, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq_recv == -1) {
        perror("mq_open (receive queue)");
        exit(EXIT_FAILURE);
    }

    mqd_t mq_sender = mq_open(other_queue, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq_sender == -1) {
        perror("mq_open (send queue)");
        mq_close(mq_recv);
        exit(EXIT_FAILURE);
    }

    char send_buf[MESSAGE_SIZE];
    char recv_buf[MESSAGE_SIZE];
    unsigned int prio;

    while (1) {
        printf("Message to %s > ", argv[2]);
        if (!fgets(send_buf, MESSAGE_SIZE, stdin))
            break;

        send_buf[strcspn(send_buf, "\n")] = '\0';

        if (strcmp(send_buf, "exit") == 0)
            break;

        if (mq_send(mq_sender, send_buf, strlen(send_buf) + 1, 1) == -1) {
            perror("mq_send");
            break;
        }

        ssize_t bytes = mq_receive(mq_recv, recv_buf, MESSAGE_SIZE, &prio);
        if (bytes == -1) {
            perror("mq_receive");
            break;
        }
        printf("Received from %s (priority %u): %s\n", argv[2], prio, recv_buf);
    }

    mq_close(mq_sender);
    mq_close(mq_recv);
    mq_unlink(my_queue);
    return 0;
}
