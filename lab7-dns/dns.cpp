#include<iostream>
#include<sys/socket.h>
#include<string.h>
#include<netdb.h>
#include<arpa/inet.h>

using std::cin;
using std::cout;
using std::endl;

void query(const char* ptr){
    struct hostent* hptr;
    char str[32];
    if((hptr=gethostbyname(ptr))==NULL){
        perror("gethostbyname error: ");
        return;
    }

    struct sockaddr_in addr;
    int i = 0;
    for(;hptr->h_addr_list[i]; ++i){
        memset(&addr, 0, sizeof(struct sockaddr_in));
        memcpy(&addr.sin_addr, hptr->h_addr_list[i], hptr->h_length);
        cout << "address: " << inet_ntop(hptr->h_addrtype, &addr.sin_addr, str, sizeof(str));
        cout << "  hptr->h_name: " << hptr->h_name << endl;
        cout << "  addr.sin_addr[" << ntohl(addr.sin_addr.s_addr) << "]";
        cout << endl;
    }
}

int main(){
    char domain[128];
    cout << "please input the domain: ";
    cin >> domain;
    query(domain);
}
