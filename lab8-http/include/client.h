#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdio.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <errno.h>
#include "db_operator.h"

//这两个函数在线程池中定义，为了方便debug这里搬了过来
//设置描述符非阻塞

int setnonblocking(int fd){
    int oldval = fcntl(fd,F_GETFL);
    int newval = oldval | O_NONBLOCK;
    fcntl(fd,newval,F_SETFL);
    return oldval;
}

//将事件添加到监听列表，并设置边沿触发模式以及同一时刻只能由一个线程处理
void addfd(int epollfd, int fd, int one_shot){
    struct epoll_event event; 
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(one_shot){
        event.events |= EPOLLRDHUP;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}


void send_file(int connfd, const char* path);
void execute_cgi(int connfd, char* data,const char* path, const char* method, char* args);

//解析HTTP请求，这里先简单处理，后期有时间改用有限状态机
void* accept_request(void* arg){
    int connfd = *((int*)arg);
    int n;
    int i;
    char c = '\0';
    struct stat st; //用于保存文件信息
    char head[100];
    char buf[2048]; //用于读取http请求报文
    char url[200];  //存储请求文件路径
    char path[200]; //存储服务器中真实的文件路径
    char method[20];//存储请求方法
    int cgi = 0;    //用来标志客户请求是否为一个cgi程序
    char* args;
    
    //这里还要改，因为这样读老是发生报文粘包，导致无法分理处POST报文的主体部分
    //啊啊啊啊啊啊终于知道是什么原因了，报文首部和主体部分有\0，所以其实数据有读入buf，只是
    //所有处理函数都默认到了\0就以为数据已经完结了
    n = recv(connfd,buf,2048,0);
    buf[n] = '\n';
    printf("%s",buf);
    //分离出报文头部起始行
    for(int i=0; i<100; ++i){
        if(buf[i]!='\n')
            head[i] = buf[i];
        else
            break;
    }
    
    if(n>0){ 
        int i=0, j=0;
        //分离http请求方法
        while(!isspace(head[i]) && j<sizeof(method)){
            method[j++] = head[i++];
        }
        method[i] = '\0';
        //这里需要注意http的请求方法是大小写都可以的，不能直接上strcmp
        if(strcasecmp(method,"GET") && strcasecmp(method,"POST")){
            //如果不是get和post则没法处理，直接返回错误
            sprintf(buf,"HTTP/1.1 501 Method Not Implemented\r\n"); //起始行
            write(connfd,buf,strlen(buf));
            sprintf(buf,"Content-Type: text/html\r\n");//头部
            write(connfd,buf,strlen(buf));
            //主体部分响应一个出错的html页面
            send_file(connfd,"./error/501.html");
            close(connfd);
            pthread_exit(NULL);
        }

        if(strcasecmp(method, "POST") == 0){
            //如果是post请求则打开cgi
            cgi = 1;
        }
        
        //提取url
        i = 0;
        while(isspace(head[j]) && j<sizeof(head))
            ++j;
        while(i<sizeof(url)-1 && j<sizeof(head) && !isspace(head[j]))
            url[i++] = head[j++];
        url[i] = '\0';
        
        //处理get请求
        if (strcasecmp(method, "GET") == 0){
            args = url;
            //get请求会把参数放在?之后
            while(*args!='?' && *args!='\0'){
                ++args;
            }   
            //如果有参数就开启cgi
            if(*args=='?'){
                cgi = 1;
                *args++ = '\0';
            }
        }

        //将url格式化到path
        //响应文件统一放在file文件夹中
        sprintf(path,"./file%s",url);
        //默认为index.html
        if(path[strlen(path)-1] == '/')
            strcat(path,"index.html");
        //根据路径寻找相应文件
        
        if (stat(path, &st) == -1){
            //回应客户端找不到 
            sprintf(buf,"HTTP/1.1 404 Not Found\r\n"); //起始行
            write(connfd,buf,strlen(buf));
            sprintf(buf,"Content-Type: text/html\r\n");//头部
            write(connfd,buf,strlen(buf));
            send_file(connfd,"./error/404.html");
            close(connfd);
            pthread_exit(NULL);
        }else{
            //如果是个目录，则默认使用该目录下的index.html
            if ((st.st_mode & S_IFMT) == S_IFDIR)
                strcat(path, "/index.html");
            if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)    )
                cgi = 1;
            //不是cgi,直接把服务器文件返回，否则执行 cgi 
            
            if (!cgi){
                send_file(connfd,path);
            }else{
                execute_cgi(connfd,buf,path,method,args);
            }
        }

    }

    //HTTP1.0默认无连接，HTTP1.1加入了长连接
   close(connfd);
}

//响应成功的报文
//这个也可以考虑用共享内存提前映射，用空间换时间
void send_file(int connfd, const char* path){
    int filefd;
    char buf[1024];
    int n;

    //发送相应头部起始行
    bzero(&buf,sizeof(buf));
    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    send(connfd,buf,strlen(buf),0);    
    //发送首部
    sprintf(buf, "connection: close\r\n\r\n");
    send(connfd,buf,strlen(buf),0);

    //发送相应主体
    filefd = open(path,O_RDONLY);
    assert(filefd>0);
    
    struct stat stat_buf;
    fstat(filefd,&stat_buf);
    sendfile(connfd,filefd,NULL,3000);
    //sendfile(connfd,filefd,NULL,stat_buf.st_size);
    //while((n=read(filefd,buf,1024))>0){
    //    write(connfd,buf,n);
    //}
    
    close(filefd);
}

//处理cgi请求
void execute_cgi(int connfd, char* data,const char* path, const char* method, char* args){
    char buf[1024];
    int j=0;
    char c;
    int ContentLength = -1;
    int cgi_input[2]; //输入管道
    int cgi_output[2];//输出管道
    if (strcasecmp(method, "POST") == 0){
        int len = strlen(data);
        for(int i=0; i<len; ++i){
            if(data[i]!='\n')
                buf[j++] = data[i];
            else{
                buf[15] = '\0';
                j = 0;
                if (strcasecmp(buf, "Content-Length:") == 0){
                    ContentLength = atoi(&buf[16]);
                    break;
                }
            }
        }

        if(ContentLength==-1){
            send_file(connfd,"./error/404.html");
            return;
        }
        //printf("%s",data);
        //int n = recv(connfd,buf,ContentLength,0);
        len = strlen(data);
        //printf("%d\n",len);
        //printf("%d\n",ContentLength);
        char post_args[ContentLength+1];
        
        for(j=0; j<ContentLength; ++j){
            post_args[j] = data[len-ContentLength+j-1];
        }
        post_args[j] = '\0';
        
        //分离参数，既然是注册，那就应该写进数据库 
        char user[10];
        char pwd[10];
        char sex[10];
        int i=0;
        //printf("%s\n",post_args);
        char* p = strtok(post_args,"&");
        while(p!=NULL){
            //printf("%s\n",p);
            if(i == 0)
                strcpy(user,p+5);
            else if(i == 1)
                strcpy(pwd,p+4);
            else if(i == 2)
                strcpy(sex,p+4);
            p = strtok(NULL,"&");
            ++i;
        }
        //printf("%d %d %d\n",strlen(user),strlen(pwd),strlen(sex));        
        //printf("%s %s %s\n",user,pwd,sex);
        if(s_insert(user,pwd,sex)==0){
            send_file(connfd,"./file/index.html");
        }else{
            send_file(connfd,"./file/register.html");
        }
        //printf("%s\n",post_args);
        //printf("%d\n",n);
        //sprintf(buf, "HTTP/1.1 200 OK\r\n");
        //write(connfd,buf,strlen(buf));
        //printf("%s\n",buf);
    }else{
        char user[10];
        char pwd[10];
        char sex[10];
        int i=0;
        //printf("%s\n",args);
        sprintf(buf, "HTTP/1.1 200 OK\r\n");
        write(connfd,buf,strlen(buf));
        char* p = strtok(args,"&");
        while(p!=NULL){
            //printf("%s\n",p);
            if(i == 0)
                strcpy(user,p+5);
            else if(i == 1)
                strcpy(pwd,p+4);
            else if(i == 2)
                strcpy(sex,p+4);
            p = strtok(NULL,"&");
            ++i;
        }
        //这里可以连接数据库校验账号和密码
        //printf("user:%s, pwd:%s, sex:%s\n",user,pwd,sex);
        //if(strcmp(user,"123") || strcmp(pwd,"123") || strcmp(sex,"male")){
            //账号或密码错误，刷新界面
        //    send_file(connfd,"./file/index.html");
        //}else{
        //    send_file(connfd,"./file/successful.html");
        //}
        
        //启用sqlite3
        //s_insert("123","123","male");
        if(s_search(user,pwd,sex)==0){
            send_file(connfd,"./file/successful.html");
        }else{
            send_file(connfd,"./file/index.html");
        }
    }
}


#endif
