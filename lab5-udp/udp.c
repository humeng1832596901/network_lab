#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<assert.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>

#define PORT 8888

void echo_func(int sockfd){
    int n;
    char buf[BUFSIZ];
    struct sockaddr_in cliaddr = {0};
    socklen_t len = sizeof(cliaddr); 
    for(;;){
        n = recvfrom(sockfd,buf,BUFSIZ,0,(struct sockaddr*)&cliaddr,&len);
        buf[n] = '\0';
        for(int i=0; i<n; ++i)
            buf[i] = toupper(buf[i]);
        sendto(sockfd,buf,n,0,(struct sockaddr*)&cliaddr,len);
    }
}

int main(void){
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    assert(sockfd>0);

    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));

    //绑定IP和监听端口号
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int ret = bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    assert(ret!=-1);

    echo_func(sockfd);
    close(sockfd);
    return 0;
}
