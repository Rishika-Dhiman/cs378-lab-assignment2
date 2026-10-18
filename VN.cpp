#include <iostream>
#include<map>
#include<vector>
#include<string>
#include<ctime>

#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<unistd.h>

using namespace std;

struct Neighbour{
    char id ;
    int ip;
    int port;
    int d;
}

struct LinkPkt{
    char originID;
    int seq;
    vector<Neighbour> neighbours;
}
int main(int argc, char** argv)
{
    in_addr on_ip, vn_ip;
    inet_pton(AF_INET,argv[1],&on_ip);
    int vn_port=atoi(argv[2]);
    sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(5000);
    addr.sin_addr=on_ip;

    int temp_fd=socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in temp;
    socklen_t temp_len= sizeof temp;
    connect(temp_fd, (sockaddr*)& addr, sizeof addr);
    getsockname(temp_fd, (sockaddr*)& temp, &temp_len);
    close(temp_fd);
    vn_ip=temp.sin_addr;

    udp_fd=socket(AF_INET, SOCK_DGRAM,0);
    sockaddr_in vn_sock;
    memset(&vn_sock, 0, sizeof vn_sock);
    vn_sock.sin_family=AF_INET;
    vn_sock.sin_port=htons(vn_port);
    vn_sock.sin_addr=vn_ip;
    bind(udp_fd, (sockaddr*)& vn_sock, sizeof vn_sock);
}