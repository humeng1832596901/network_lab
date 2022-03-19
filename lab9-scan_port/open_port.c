#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<time.h>

int main(void){
    srand(time(NULL));
    int open_ports[5];
    int sockfds[5];
    int flag=1;
    for(int i=0; i<5; ++i){
        open_ports[i] = rand()%10000 + 50000;
        sockfds[i] = socket(AF_INET,SOCK_STREAM,0);
        struct sockaddr_in servaddr;
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(open_ports[i]);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        setsockopt(sockfds[i],SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

        bind(sockfds[i],(struct sockaddr*)&servaddr,sizeof(servaddr));
        listen(sockfds[i],5);      
    }
    for(int i=0; i<5; ++i){
        printf("listen port: %d\n",open_ports[i]);
    }
    while(1);
}
