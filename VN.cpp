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