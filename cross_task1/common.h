#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define MAX_PACKET_SIZE 65536
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define EXIT_CMD "CLOSE_CONNECTION"
#define IP_VERSION 4
#define IP_IHL 5
#define IP_TTL 255
#define IP_ID 54321

struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

static inline unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    short answer;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (short)~sum;

    return answer;
}

#endif
