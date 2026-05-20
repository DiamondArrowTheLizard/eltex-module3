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

#define MAX_CLIENTS 1024

struct client_state {
    uint32_t ip;
    uint16_t port;
    int message_count;
    int active;
};

static struct client_state clients[MAX_CLIENTS];
static int server_socket;

void handle_signal(int sig) {
    (void)sig;
    close(server_socket);
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    server_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    int one = 1;
    if (setsockopt(server_socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    memset(clients, 0, sizeof(clients));

    unsigned char buffer[MAX_PACKET_SIZE];
    struct sockaddr_in s_addr;
    socklen_t s_len = sizeof(s_addr);

    while (1) {
        ssize_t data_len = recvfrom(server_socket, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&s_addr, &s_len);
        if (data_len < 0) continue;

        struct iphdr *ip = (struct iphdr *)buffer;
        if (ip->protocol != IPPROTO_UDP) continue;

        struct udphdr *udp = (struct udphdr *)(buffer + (ip->ihl * 4));
        if (ntohs(udp->dest) != SERVER_PORT) continue;

        char *payload = (char *)(buffer + (ip->ihl * 4) + sizeof(struct udphdr));
        int payload_len = ntohs(udp->len) - sizeof(struct udphdr);
        payload[payload_len] = '\0';

        uint32_t client_ip = ip->saddr;
        uint16_t client_port = udp->source;

        int client_idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && clients[i].ip == client_ip && clients[i].port == client_port) {
                client_idx = i;
                break;
            }
        }

        if (strcmp(payload, EXIT_CMD) == 0) {
            if (client_idx != -1) {
                clients[client_idx].active = 0;
            }
            continue;
        }

        if (client_idx == -1) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i].active) {
                    clients[i].ip = client_ip;
                    clients[i].port = client_port;
                    clients[i].message_count = 0;
                    clients[i].active = 1;
                    client_idx = i;
                    break;
                }
            }
        }

        if (client_idx == -1) continue;

        clients[client_idx].message_count++;

        char response_payload[MAX_PACKET_SIZE];
        int resp_len = snprintf(response_payload, sizeof(response_payload), "%s %d", payload, clients[client_idx].message_count);

        unsigned char send_buffer[MAX_PACKET_SIZE];
        memset(send_buffer, 0, MAX_PACKET_SIZE);

        struct iphdr *rip = (struct iphdr *)send_buffer;
        struct udphdr *rudp = (struct udphdr *)(send_buffer + sizeof(struct iphdr));
        char *rdata = (char *)(send_buffer + sizeof(struct iphdr) + sizeof(struct udphdr));

        memcpy(rdata, response_payload, resp_len);

        rip->ihl = 5;
        rip->version = 4;
        rip->tos = 0;
        rip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + resp_len);
        rip->id = htons(12345);
        rip->frag_off = 0;
        rip->ttl = 255;
        rip->protocol = IPPROTO_UDP;
        rip->check = 0;
        rip->saddr = ip->daddr;
        rip->daddr = ip->saddr;
        rip->check = calculate_checksum((unsigned short *)send_buffer, sizeof(struct iphdr));

        rudp->source = udp->dest;
        rudp->dest = udp->source;
        rudp->len = htons(sizeof(struct udphdr) + resp_len);
        rudp->check = 0;

        struct pseudo_header psh;
        psh.source_address = rip->saddr;
        psh.dest_address = rip->daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_UDP;
        psh.udp_length = rudp->len;

        int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + resp_len;
        unsigned char *pseudogram = malloc(psize);
        memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
        memcpy(pseudogram + sizeof(struct pseudo_header), rudp, sizeof(struct udphdr) + resp_len);
        rudp->check = calculate_checksum((unsigned short *)pseudogram, psize);
        free(pseudogram);

        struct sockaddr_in d_addr;
        d_addr.sin_family = AF_INET;
        d_addr.sin_port = rudp->dest;
        d_addr.sin_addr.s_addr = rip->daddr;

        sendto(server_socket, send_buffer, ntohs(rip->tot_len), 0, (struct sockaddr *)&d_addr, sizeof(d_addr));
    }

    return 0;
}
