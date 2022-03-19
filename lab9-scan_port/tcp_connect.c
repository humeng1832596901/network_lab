#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;

    if (argc != 4) {
        printf("usage: fulfill the cmd\n");
        return -1;
    }

    int i;
    for (i = atoi(argv[2]); i < atoi(argv[3]); i++) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            return -1;
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(i);

        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
            printf("inet_pton error\n");
        }

        if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
            close(sockfd);
            continue;
        }
        else {
            printf("useful port: %d\n", i);
            close(sockfd);
            continue;
        }
    }
    return 0; 
}
