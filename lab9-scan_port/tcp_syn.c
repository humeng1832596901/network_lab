#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<linux/tcp.h>
#include<string.h>
#include<netinet/ip.h>
#include<sys/wait.h>
#include<unistd.h>

unsigned short check_sum(unsigned short *buffer, int size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof (unsigned short);
    }
    if (size) {
        cksum += *(unsigned char*) buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return (unsigned short) (~cksum);
}


void send_data(int sockfd, struct sockaddr_in *addr, int sourceport, char sourceip[30]) {
    char buffer[100];
    struct iphdr *ip;
    struct tcphdr *tcp;

    int head_len;
    int n, i;
    u_char * pPseudoHead;
    u_char pseudoHead[12 + sizeof (struct tcphdr) ];
    u_short tcpHeadLen;

    tcpHeadLen = htons(sizeof (struct tcphdr));
    head_len = sizeof (struct iphdr) + sizeof (struct tcphdr);
    bzero(buffer, 100);

    //构建ip头
    ip = (struct iphdr *) buffer;
    ip->version = IPVERSION;
    ip->ihl = sizeof (struct ip) >> 2;
    ip->tos = 0;
    ip->tot_len = htons(head_len);
    ip->id = 0;
    ip->frag_off = 0;
    ip->ttl = MAXTTL;
    ip->protocol = IPPROTO_TCP;
    ip->check = 0;
    ip->daddr = addr->sin_addr.s_addr;
    ip->saddr = inet_addr(sourceip);


    //构建TCP头
    tcp = (struct tcphdr *) (buffer + sizeof (struct ip));
    tcp->source = htons(sourceport);
    tcp->dest = addr->sin_port;
    tcp->seq = htonl(30000);
    tcp->ack_seq = 0;
    tcp->doff = 5;
    tcp->syn = 1;
    tcp->urg_ptr = 0;
    tcp->window = htons(10052);


    //构建伪首部
    pPseudoHead = pseudoHead;
    memset(pPseudoHead, 0, 12 + sizeof (struct tcphdr));
    memcpy(pPseudoHead, &ip->saddr, 4);
    pPseudoHead += 4;
    memcpy(pPseudoHead, &ip->daddr, 4);
    pPseudoHead += 4;
    memset(pPseudoHead, 0, 1);
    pPseudoHead++;
    memset(pPseudoHead, 0x0006, 1);
    pPseudoHead++;
    memcpy(pPseudoHead, &tcpHeadLen, 2);
    pPseudoHead += 2;
    memcpy(pPseudoHead, tcp, sizeof (struct tcphdr));

    tcp->check = 0;
    tcp->check = check_sum((unsigned short *) pseudoHead, sizeof (struct tcphdr) + 12);
    if (sendto(sockfd, buffer, head_len, 0, (struct sockaddr *) addr, (socklen_t)sizeof (struct sockaddr_in)) < 0) {
        perror("sendto");
    }
}

void  recv_packet(const char* localIP, int localPort, int sockfd, int startport, int endport) {
    struct tcphdr * tcp;
    char *srcaddr;
    int loopend;
    int size;
    char readbuff[1600];
    struct sockaddr_in from;
    int from_len,n;

    tcp = (struct tcphdr *) (readbuff + 20); /*那个sockfd中读出的数据包括了IP头的所以+20*/
    for (n = startport; n < endport + 1; n++) {
        size = recv(sockfd, readbuff, 1600, MSG_DONTWAIT);
        if (size < (20 + 20))/*读出的数据小于两个头的最小长度的话continue*/
        continue;
        if (ntohs(tcp->dest) != localPort)
        continue;
        if (tcp->rst && tcp->ack)/*端口关闭或者没有服务*/
        continue;
        if (tcp->ack && tcp->syn)/*端口开启*/ {
            printf("%5u open\n", (ntohs(tcp->source)));
            fflush(stdout);
            continue;
        }
    }
}
int main(int argc, char** argv){
    if(argc!=4){
        return -1;
    }
    char ip[30];
    int myport = 8888;
    char* myip = "192.168.145.128";
    int flag = 1;
    strcpy(ip,argv[1]);
    int startport = atoi(argv[2]);
    int endport = atoi(argv[3]);
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&addr.sin_addr);
    int sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
    setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&flag,sizeof(flag));
    for(int i=startport; i<=endport; ++i){
        addr.sin_port = htons(i);
        send_data(sockfd,&addr,myport,myip);
        recv_packet(myip,myport,sockfd,startport,endport);
    }
    return 0;
}
