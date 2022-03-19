/*************************************************************************
> File Name: arp.c
> Author: 
> Mail: 
> Created Time: Mon 19 Oct 2020 05:54:06 PM CST
************************************************************************/

#include "wrap.h"

#define INLEN 4
#define MAC_BCAST_ADDR  (uint8_t *) "/xff/xff/xff/xff/xff/xff"

struct arp_packet{
    struct ether_header eh;
    struct ether_arp ea;
    u_char padding[18];
}req;

int get_ifi(char *dev, char * mac, int macln, struct in_addr *lc_addr, int ipln){
    int reqfd, n;
    struct ifreq macreq;

    reqfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(macreq.ifr_name, dev);

    /* 获取本地接口MAC地址*/
    Ioctl(reqfd, SIOCGIFHWADDR, &macreq);

    memcpy(mac, macreq.ifr_hwaddr.sa_data, macln);

    /* 获取本地接口IP地址*/
    Ioctl(reqfd, SIOCGIFADDR, &macreq);

    memcpy(lc_addr, &((struct sockaddr_in *)(&macreq.ifr_addr))->sin_addr, ipln);

    return 0;
}      

void prmac(u_char *ptr){
    printf("Peer MAC is: %02x:%02x:%02x:%02x:%02x:%02x\n",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
}

int main(int argc, char** argv){
    if(argc!=2){
        printf("Usage: %s <query_IP>\n",argv[0]);
    }
    int reqfd, recvfd, salen, n;  
    u_char* mac;
    char recv_buf[BUFSIZ], rep_addr[BUFSIZ];
    struct in_addr req_addr, lc_addr;
    struct sockaddr_ll reqsa, repsa;
    bzero(&reqsa,sizeof(reqsa));
    reqsa.sll_family = PF_PACKET;
    reqsa.sll_ifindex = if_nametoindex("eth0");
    reqfd = Socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
    mac = (char *)malloc(ETH_ALEN);
    bzero(&req, sizeof(req));
    get_ifi("eth0", mac, ETH_ALEN, &lc_addr, INLEN);
    /* 填写以太网头部*/
    memcpy(req.eh.ether_dhost, MAC_BCAST_ADDR, ETH_ALEN);
    memcpy(req.eh.ether_shost, mac, ETH_ALEN);
    req.eh.ether_type = htons(ETHERTYPE_ARP);

    /* 填写arp数据 */
    req.ea.arp_hrd = htons(ARPHRD_ETHER);
    req.ea.arp_pro = htons(ETHERTYPE_IP);
    req.ea.arp_hln = ETH_ALEN;
    req.ea.arp_pln = INLEN;
    req.ea.arp_op = htons(ARPOP_REQUEST);
    memcpy(req.ea.arp_sha, mac, ETH_ALEN);
    memcpy(req.ea.arp_spa, &lc_addr, INLEN);
    inet_aton(argv[1], (struct in_addr*)&req.ea.arp_tpa);
    n = Sendto(reqfd, &req, sizeof(req), 0, (struct sockaddr *)&reqsa, sizeof(reqsa));
    printf("Broadcast arp request of %s, %d bytes be sent\n\n", argv[1], n);

    recvfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    bzero(recv_buf, sizeof(recv_buf));
    bzero(&repsa, sizeof(repsa));
    salen = sizeof(struct sockaddr_ll);

    while(1) {
        if((n = recvfrom(recvfd, recv_buf, sizeof(req), 0, (struct sockaddr *)&repsa, &salen)) <= 0) {
            perror("Recvfrom error");
            exit(1);
        }

        if( ntohs(*(__be16 *)(recv_buf + 20))==2 && !memcmp(req.ea.arp_tpa, recv_buf + 28, 4) ) {
            printf("Response from %s, %d bytes received\n", argv[1], n);
            printf("Peer IP is: %s\n", inet_ntop(AF_INET, (struct in_addr *)(recv_buf + 28), rep_addr, 1024));
            prmac( (u_char *)(recv_buf + 22) ); 
            break;
        }
    }
    free(mac);
    return 0;
}
