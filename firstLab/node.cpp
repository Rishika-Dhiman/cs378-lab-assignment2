#include <iostream>
#include<string>
#include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment(lib, "ws2_32.lib");


int main(int argc, char* argv[]){
    char node_name = argv[1][0];
    int port = stoi(argv[2]);
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    bind(sock_fd, (sockaddr*)&addr, sizeof(addr));
    cout<<node_name<<":"<<port<<endl;
    if (node_name == 'A'){
        char buffer[1024];
        sockaddr_in sender{};
        int sender_len = sizeof(sender);
        int bytes_rcvd = recvfrom(sock_fd, buffer, sizeof(buffer)-1, 0, (sockaddr*)&sender, &sender_len);
        buffer[bytes_rcvd] = '\0';
        cout<<"received:"<<buffer<<endl;
    }
    else{
        sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(5000);
        inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);
        string msg = "Node B here!";
        int bytes_sent = sendto(sock_fd, msg.c_str(), msg.length(), 0, (sockaddr*)&dest, sizeof(dest));
        cout<<"sent"<<endl;
    }
    closesocket(sock_fd);
    WSACleanup();
}