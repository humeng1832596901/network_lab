#include "db_operator.h"
#include "client.h"
#include "threadpool.h"

#define MAX_EVENT_NUMBER 10000

int main(int arg, char** argv){
    int PORT = 8888;
    char IP[20] = "192.168.145.128";
    if(arg==3){
        PORT = atoi(argv[2]);
        strcpy(IP,argv[1]);
    }
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    int ret = -1;
    pthread_t tid[100];
    int i=0;
    //初始化线程池
    threadpool_t* thp = threadpool_create(10,1000,1000);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd>0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    ret = inet_pton(AF_INET,IP,&servaddr.sin_addr);
    assert(ret != -1);
    
    //设置端口复用
    int flag = 1;
    ret = setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    assert(ret != -1);

    ret = bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret != -1);
    printf("服务器开启...\n");
    
    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listenfd,0);

    while(1){
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if(number<0 && errno!=EINTR){
            perror("epoll error:");
            exit(-1);
        }
        for(int idx=0; idx<number; ++idx){
            int sockfd = events[idx].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in cliaddr;
                bzero(&cliaddr,sizeof(cliaddr));
                socklen_t len = sizeof(cliaddr);
                
                connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&len);
                assert(connfd);
                addfd(epollfd,connfd,1);
            }else{
                int temp = sockfd;
                threadpool_add_task(thp,accept_request,&temp);
                
            }
        }
        
    }
    close(listenfd);
    return 0;
}
