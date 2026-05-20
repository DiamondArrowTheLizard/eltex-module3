#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 65536

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port_to_sniff>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int target_port = atoi(argv[1]);

    int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);
    struct sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);

    while (1) {
        ssize_t packet_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, 
                                       (struct sockaddr *)&saddr, &saddr_len);
        if (packet_size < 0) {
            perror("recvfrom");
            break;
        }

        struct iphdr *ip_header = (struct iphdr *)buffer;
        unsigned short iphdrlen = ip_header->ihl * 4;

        struct udphdr *udp_header = (struct udphdr *)(buffer + iphdrlen);

        if (ntohs(udp_header->dest) == target_port) {
            unsigned char *data = buffer + iphdrlen + sizeof(struct udphdr);
            int data_len = packet_size - iphdrlen - sizeof(struct udphdr);

            printf("\nCaptured Packet:\n");
            printf("From: %s:%d\n", inet_ntoa(saddr.sin_addr), ntohs(udp_header->source));
            printf("Data size: %d bytes\n", data_len);
            
            if (data_len > 0) {
                printf("Content: ");
                for (int i = 0; i < data_len; i++) {
                    if (data[i] >= 32 && data[i] <= 126)
                        printf("%c", data[i]);
                    else
                        printf(".");
                }
                printf("\nHex dump: ");
                for (int i = 0; i < data_len; i++) {
                    printf("%02x ", data[i]);
                }
                printf("\n");
            }
        }
    }

    free(buffer);
    close(raw_sock);
    return 0;
}
