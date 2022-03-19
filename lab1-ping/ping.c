/*************************************************************************
> File Name: ping.c
> Author: Hu Meng
> Mail: 
> Created Time: Thu 08 Oct 2020 10:45:46 AM CST
************************************************************************/

#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<netinet/ip_icmp.h>
#include<netinet/ip.h>
#include<time.h>
#include<sys/types.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<netinet/in.h>

int sockfd;
struct sockaddr_in dst_addr;
char sendbuf[2048];
char recvbuf[2048];
unsigned int seq = 0;

//IP校验码
unsigned short checknum(unsigned char* addr, int len){
    int nleft = len;
    unsigned int sum = 0;
    unsigned short* tmp = (unsigned short*)addr;

    while(nleft > 1){
        sum += *tmp++;
        nleft -= 2;
    }

    if(nleft == 1)
    sum += *tmp;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

void icmp_pack(){
    int i;
    pid_t pid = getpid();
    bzero(sendbuf,2048);
    struct icmp* icmp = (struct icmp*)sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = pid;
    icmp->icmp_seq = seq++;
    struct timeval* tvstart;
    tvstart = (struct timeval*)&icmp->icmp_data;
    gettimeofday(tvstart,NULL);
    icmp->icmp_cksum = checknum((unsigned char*)sendbuf,64);
}

void send_pack(){
    icmp_pack();
    sendto(sockfd,sendbuf,64,0,(struct sockaddr*)&dst_addr,sizeof(dst_addr));
}

void alarm_send(int signo){
    send_pack();
    alarm(3);
    return;
}

void receive_pack(int sockfd){
    int n=0; 
    struct ip* p_ip;
    struct icmp* p_icmp;
    unsigned short len_iphead, len_icmp;
    char ip_source[16];
    unsigned short sum_recv, sum_cal;
    struct timeval *tvstart, *tvend;
    tvend = malloc(sizeof( sizeof(struct timeval)) );
    double delt_sec;
    while(1){
        bzero(recvbuf,2048);
        n = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if(n < 0){
            if(errno==EINTR)
                continue;
            else{
                perror("recvfrom error: ");
                exit(-1);
            }
        }

        gettimeofday(tvend,NULL);

        p_ip = (struct ip*)recvbuf;
        len_iphead = 4*(p_ip->ip_hl);

        len_icmp = ntohs(p_ip->ip_len)-len_iphead;
        p_icmp = (struct icmp*)((unsigned char*)p_ip + len_iphead);
        sum_recv = p_icmp->icmp_cksum;
        p_icmp->icmp_cksum = 0;

        sum_cal = checknum( (unsigned char*)p_icmp, len_icmp);
        if(sum_cal != sum_recv){
            printf("checksum error\tsum_recv = %d\tsum_cal = %d\n",sum_recv, sum_cal);
        }
        else{
            switch(p_icmp->icmp_type){
                case ICMP_ECHOREPLY:
                {
                    pid_t pid_now, pid_rev;
                    pid_rev = (p_icmp->icmp_id);
                    pid_now = getpid();
                    if(pid_rev != pid_now ){
                        printf("pid not match!pin_now = %d, pin_rev = %d\n", pid_now, pid_rev);
                    }
                    inet_ntop(AF_INET, (void*)&(p_ip->ip_src), ip_source, INET_ADDRSTRLEN);
                    tvstart = (struct timeval*)p_icmp->icmp_data;
                    delt_sec = (tvend->tv_sec - tvstart->tv_sec) + (tvend->tv_usec - tvstart->tv_usec)/1000000.0;
                    printf("%d bytes from %s: icmp_req=%d ttl=%d time=%4.2f ms\n", len_icmp, ip_source, p_icmp->icmp_seq, p_ip->ip_ttl, delt_sec*1000);
                    break;
                }
                case ICMP_TIME_EXCEEDED:
                {
                    printf("time out!\n");
                    break;
                }
                case ICMP_DEST_UNREACH:
                {
                    inet_ntop(AF_INET, (void*)&(p_ip->ip_src), ip_source, INET_ADDRSTRLEN);
                    printf("From %s icmp_seq=%d Destination Host Unreachable\n", ip_source, p_icmp->icmp_seq);
                    break;
                }
            }
        }
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("ping address\n");
        exit(-1);
    }
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd <= 0){
        perror("sock: ");
        exit(-1);
    }
    bzero(&dst_addr,sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;

    if(inet_pton(AF_INET,argv[1],&dst_addr.sin_addr) != 1){
        perror("inet_pton error: ");
        exit(-1);
    }

    struct sigaction newact, oldact;
    newact.sa_handler = alarm_send;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGALRM, &newact, &oldact);
    alarm(3);

    receive_pack(sockfd);

    return 0;
}
