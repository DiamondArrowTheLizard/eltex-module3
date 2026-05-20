#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <time.h>

static int client_socket;
static uint16_t local_port;
static volatile sig_atomic_t keep_running = 1;

void send_raw_udp(int sock, const char *msg, uint32_t saddr, uint32_t daddr, uint16_t sport) {
    unsigned char buffer[MAX_PACKET_SIZE];
    memset(buffer, 0, MAX_PACKET_SIZE);

    int msg_len = strlen(msg);
    struct iphdr *ip = (struct iphdr *)buffer;
    struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
    char *data = (char *)(buffer + sizeof(struct iphdr) + sizeof(struct udphdr));

    memcpy(data, msg, msg_len);

    ip->ihl = IP_IHL;
    ip->version = IP_VERSION;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + msg_len);
    ip->id = htons(IP_ID);
    ip->frag_off = 0;
    ip->ttl = IP_TTL;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->saddr = saddr;
    ip->daddr = daddr;
    ip->check = calculate_checksum((unsigned short *)buffer, sizeof(struct iphdr));

    udp->source = htons(sport);
    udp->dest = htons(SERVER_PORT);
    udp->len = htons(sizeof(struct udphdr) + msg_len);
    udp->check = 0;

    struct pseudo_header psh;
    psh.source_address = saddr;
    psh.dest_address = daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = udp->len;

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + msg_len;
    unsigned char *pseudogram = malloc(psize);
    memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udp, sizeof(struct udphdr) + msg_len);
    udp->check = calculate_checksum((unsigned short *)pseudogram, psize);
    free(pseudogram);

    struct sockaddr_in d_addr;
    d_addr.sin_family = AF_INET;
    d_addr.sin_port = udp->dest;
    d_addr.sin_addr.s_addr = daddr;

    sendto(sock, buffer, ntohs(ip->tot_len), 0, (struct sockaddr *)&d_addr, sizeof(d_addr));
}

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    srand(time(NULL));
    local_port = 49152 + (rand() % 16383);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (client_socket < 0) {
        perror("socket");
        exit(1);
    }

    int one = 1;
    setsockopt(client_socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    uint32_t saddr = inet_addr("127.0.0.1");
    uint32_t daddr = inet_addr(SERVER_IP);

    char input[MAX_PACKET_SIZE];
    unsigned char recv_buf[MAX_PACKET_SIZE];

    while (keep_running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(client_socket, &fds);

        struct timeval tv = {1, 0};
        if (select(client_socket + 1, &fds, NULL, NULL, &tv) > 0) {
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                if (fgets(input, sizeof(input), stdin)) {
                    input[strcspn(input, "\n")] = 0;
                    send_raw_udp(client_socket, input, saddr, daddr, local_port);
                }
            }

            if (FD_ISSET(client_socket, &fds)) {
                ssize_t len = recv(client_socket, recv_buf, MAX_PACKET_SIZE, 0);
                if (len > 0) {
                    struct iphdr *rip = (struct iphdr *)recv_buf;
                    struct udphdr *rudp = (struct udphdr *)(recv_buf + (rip->ihl * 4));

                    if (ntohs(rudp->dest) == local_port) {
                        char *data = (char *)(recv_buf + (rip->ihl * 4) + sizeof(struct udphdr));
                        int dlen = ntohs(rudp->len) - sizeof(struct udphdr);
                        data[dlen] = '\0';
                        printf("Server echo: %s\n", data);
                    }
                }
            }
        }
    }

    send_raw_udp(client_socket, EXIT_CMD, saddr, daddr, local_port);
    close(client_socket);
    return 0;
}
