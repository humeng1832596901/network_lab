/*************************************************************************
	> File Name: wrap.h
	> Author: 
	> Mail: 
	> Created Time: Mon 19 Oct 2020 05:59:24 PM CST
 ************************************************************************/

#ifndef _WRAP_H
#define _WRAP_H

#include<stdio.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<sys/un.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<net/if.h>
#include<errno.h>
#include<stdlib.h>
#include<net/if_arp.h>
#include<netpacket/packet.h>
#include<net/ethernet.h>
#include<arpa/inet.h>
#include<netinet/if_ether.h>

void sys_error(char* str){
    perror(str);
    exit(1);
}

int Socket(int domain, int type, int protocol){
    int ret = socket(domain,type,protocol);
    if(ret==-1)
        sys_error("socket error:");
    else
        return ret;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen){
   ssize_t ret = sendto(sockfd,buf,len,flags,(const struct sockaddr*)dest_addr,addrlen);
    if(ret<=0)
        sys_error("sendto error:");
    else
        return ret;
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen){
    ssize_t ret = recvfrom(sockfd,buf,len,flags,src_addr,addrlen);
    if(ret<=0)
        perror("recvfrom error:");
    else
        return ret;
}

int Ioctl(int fd, unsigned long request, struct ifreq* macreq){
    int ret = ioctl(fd,request,macreq);
    if(ret!=0)
        sys_error("get host info error:");
    return ret;
}

#endif
