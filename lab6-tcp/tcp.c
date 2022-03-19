#include "wrap.h"
#include <ctype.h>

#define IP "192.168.220.128"
#define PORT 8888
#define MAX_EVENT_NUM 1024

int set_nonblock(int fd){
    int oldval = fcntl(fd,F_GETFL);
    int newval = oldval | O_NONBLOCK;
    fcntl(fd,F_SETFL,newval);
    return oldval;
}

void addfd(int epollfd, int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    set_nonblock(fd);
}

void getsockip(int sockfd,char* temp,size_t buf_size){
    struct sockaddr_in cliaddr;
    bzero(&cliaddr,sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);
    Getsockname(sockfd,(struct sockaddr*)&cliaddr,&len);
    Inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,temp,buf_size);
}

int main(void){
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    Inet_pton(AF_INET,IP,&servaddr.sin_addr);

    //TCP socket
    int tcpfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(tcpfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    Listen(tcpfd,128);

    //UDP socket
    int udpfd = Socket(AF_INET,SOCK_DGRAM,0);
    Bind(udpfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

    struct epoll_event events[MAX_EVENT_NUM];
    int epollfd = Epoll_create(128);
    addfd(epollfd,tcpfd);
    addfd(epollfd,udpfd);
    int connfdset[MAX_EVENT_NUM];
    int idx = 0;
    
    char buf[BUFSIZ];
    for(;;){
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUM,-1);
        for(int i=0; i<number; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd==tcpfd){
                struct sockaddr_in cliaddr;
                bzero(&cliaddr,sizeof(cliaddr));
                socklen_t len = sizeof(cliaddr);
                int connfd = Accept(tcpfd,(struct sockaddr*)&cliaddr,&len);
                addfd(epollfd,connfd);
                connfdset[idx++] = connfd;
            }else if(sockfd==udpfd){
                memset(buf,'\0',BUFSIZ);
                char cli_ip[100];
                struct sockaddr_in cliaddr;
                bzero(&cliaddr,sizeof(cliaddr));
                socklen_t len = sizeof(cliaddr);
                int n = Recvfrom(sockfd,buf,BUFSIZ,0,(struct sockaddr*)&cliaddr,&len);
                //Inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,cli_ip,100);
                if(n<0){
                    exit(1);
                }else if(n>0){
                    for(int i=0; i<n; ++i)
                        buf[i] = toupper(buf[i]);
                    buf[n] = '\0';
                    Sendto(sockfd,buf,n,0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
                }else{
                    printf("one udp_client is closed\n");
                }
            }else if(events[i].events & EPOLLIN){
                memset(buf,'\0',BUFSIZ);
                char cli_ip[100];
                struct sockaddr_in cliaddr;
                bzero(&cliaddr,sizeof(cliaddr));
                socklen_t len = sizeof(cliaddr);
                int n = recvfrom(sockfd,buf,BUFSIZ,0,(struct sockaddr*)&cliaddr,&len);
                //Inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,cli_ip,100);
                if(n<0)
                    exit(1);
                else if(n>0){
                    for(int i=0; i<n; ++i)
                        buf[i] = toupper(buf[i]);
                    buf[n] = '\0';
                    write(sockfd,buf,n);
                }else{
                    printf("one tcp_client is closed\n");
                }
            }else{
                printf("something else happened\n");
            }
        }
    }
    close(tcpfd);
    close(udpfd);
    return 0;
}
