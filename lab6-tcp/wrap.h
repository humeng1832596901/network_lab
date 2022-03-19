#ifndef _WRAP_H
#define _WRAP_H

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<pthread.h>
#include<string.h>

void sys_err(char* str){
    perror(str);
    exit(1);
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
    ssize_t ret = sendto(sockfd,buf,len,flags,dest_addr,addrlen);
    if(ret != -1)
        return ret;
    else{
        if(errno==EMSGSIZE)
            sys_err("the size of message is invalid:");
        else
            sys_err("sendto error:");
    }
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    ssize_t ret = recvfrom(sockfd,buf,len,flags,src_addr,addrlen);
    if(ret != -1)
        return ret;
    else
        sys_err("recvfrom error:");
}

int Socket(int domain, int type, int protocol){
    int ret = socket(domain,type,protocol);
    if(ret != -1)
        return ret;
    else
        sys_err("socket error: ");
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret = bind(sockfd,addr,addrlen);
    if(ret != -1)
        return ret;
    else
        sys_err("bind error: ");
}

int Listen(int sockfd, int backlog){
    int ret = listen(sockfd,backlog);
    if(ret==-1)
        sys_err("listen error:");
    else
        return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret = accept(sockfd,addr,addrlen);
    if(ret==-1)
        sys_err("accept error:");
    else
        return ret;
}

int Inet_pton(int af, const char *src, void *dst){
    int ret = inet_pton(af,src,dst);
    if(ret != -1)
        return ret;
    else
        sys_err("inet_pton error:");
}

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size){
    const char* ret = inet_ntop(af,src,dst,size);
    if(ret != NULL)
        return ret;
    else
        sys_err("inet_ntop error: ");
}

int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen){
    int ret = connect(sockfd,addr,addrlen);
    if(ret != -1)
        return ret;
    else
        sys_err("connect error:");
}

int Epoll_create(int size){
    int ret = epoll_create(size);
    if(ret==-1)
        sys_err("epoll_create error:");
    else
        return ret;
}

int Epoll_wait(int epfd, struct epoll_event *events,
        int maxevents, int timeout){
    int ret = epoll_wait(epfd,events,maxevents,timeout);
    if(ret==-1)
        sys_err("epoll_wait error:");
    else
        return ret;
}

int Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret = getsockname(sockfd,addr,addrlen);
    if(ret==0)
        return ret;
    else
        sys_err("getsockname error:");
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
    ssize_t ret = sendto(sockfd,buf,len,flags,dest_addr,addrlen);
    if(ret!=-1){
        return ret;
    }else{
        sys_err("sendto error: ");
        exit(1);
    }
}

#endif
