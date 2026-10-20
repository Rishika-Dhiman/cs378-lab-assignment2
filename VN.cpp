#include <iostream>
#include<map>
#include<vector>
#include<cstring>
#include<ctime>
#include<cstdint>
#include <cstdlib>

#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<unistd.h>

using namespace std;

struct Nbr{
    char id ;
    uint32_t ip;
    uint16_t port;
    int16_t d;
};

struct Lsp{
    char originID;
    uint32_t seq;
    vector<Nbr> nbrs;
};

int udp_fd, tcp_fd;
bool first_rcvd = false;
char vn_id = '#';
uint32_t vn_seq = 0;
map<char, Lsp> ls_map;
vector<Nbr> vn_nbrs;
vector<uint8_t> buffer, last_msg;
time_t next_lsp=0;

void send_to_all(const vector<uint8_t>& pkt, sockaddr_in* src){
    
    for (int i = 0; i < (int)vn_nbrs.size(); i++){
        sockaddr_in nbr_sock;
        memset(&nbr_sock, 0, sizeof nbr_sock);
        nbr_sock.sin_family = AF_INET;
        nbr_sock.sin_port = htons(vn_nbrs[i].port);
        nbr_sock.sin_addr.s_addr = vn_nbrs[i].ip;
        if(src &&src->sin_addr.s_addr == nbr_sock.sin_addr.s_addr && src->sin_port == nbr_sock.sin_port) continue;
        sendto(udp_fd, pkt.data(), pkt.size(), 0, (sockaddr*)&nbr_sock, sizeof nbr_sock);
    }
    cout<<"VN id "<<vn_id<<" sending LS pkts"<<endl;
}

void new_vn_lsp(){
    Lsp lsp;
    lsp.seq=++vn_seq;
    lsp.nbrs=vn_nbrs;
    ls_map[vn_id]=lsp;
    vector<uint8_t> p;
    p.push_back(vn_id);
    for(int i=24;i>=0;i-=8)
    {
        p.push_back((lsp.seq>>i) & 0xff);
    }
    p.push_back(lsp.nbrs.size());
    for(int i=0;i<lsp.nbrs.size();i++)
    {
        Nbr& n=lsp.nbrs[i];
        p.push_back(n.id);
        uint8_t* ip=(uint8_t*)&n.ip;
        p.insert(p.end(),ip,ip+4);
        p.push_back(n.port>>8);
        p.push_back(n.port & 0xff);
        p.push_back((uint16_t)n.d>>8);
        p.push_back((uint16_t)n.d & 0xff);
    }
    send_to_all(p,NULL);
    next_lsp=time(NULL)+15;
}


void tcp_lsp()
{
    uint8_t buff[512];
    int l=recv(tcp_fd, buff, sizeof buff, 0);
    if(l<=0)
    {
        close(tcp_fd);
        tcp_fd=-1;
        return;
    }
    buffer.insert(buffer.end(), buff, buff + l);
    while(!buffer.empty()){
        size_t n = buffer[0], total = 1 + 9 * n;
        if (buffer.size() < total) break;
        vector<uint8_t> msg(buffer.begin(), buffer.begin() + total);
        buffer.erase(buffer.begin(), buffer.begin() + total);
        if (msg == last_msg) continue;
        last_msg = msg;

        vn_nbrs.clear();
        for (size_t i = 0; i < n; i++){
            uint8_t* p = &msg[1 + 9 * i];
            Nbr e;
            e.id = p[0];
            memcpy(&e.ip, p+1, 4);
            e.port = ((p+5)[0] << 8) | (p+5)[1];
            e.d = (int16_t)((p+7)[0] << 8) | (p+7)[1];
            if (e.d == 0) vn_id = e.id;
            else vn_nbrs.push_back(e);
        }
        cout<<"VN id "<<vn_id<<" rcvd msg from ON"<<endl;
        first_rcvd = true;
        new_vn_lsp();
    }

}

void udp_lsp(uint8_t* buff, size_t l, sockaddr_in& src){
    if (l < 6 || l != 6 + 9 * (size_t)buff[5]) return;
    char cur_id = buff[0];
    if (cur_id < 'A' || cur_id > 'Z') return;
    Lsp lsp;
    lsp.seq = ((uint32_t)buff[1] << 24) | (buff[2]<<16) | (buff[3] <<8) | buff[4];
    for (size_t i = 0; i < buff[5]; i++){
        uint8_t* p = buff + 6 + i*9;
        Nbr e;
        e.id = p[0];
        memcpy(&e.ip, p+1, 4);
        e.port = ((p+5)[0] << 8) | (p+5)[1];
        e.d = (int16_t)((p+7)[0] << 8) | (p+7)[1];
        lsp.nbrs.push_back(e);
    }
    map<char, Lsp>::iterator it = ls_map.find(cur_id);
    if (it != ls_map.end() && lsp.seq <= it->second.seq) return;
    ls_map[cur_id] = lsp;
    send_to_all(vector<uint8_t>(buff, buff + l), &src);
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

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(tcp_fd, (sockaddr*)&addr, sizeof addr);
    vector<uint8_t> connect_msg;
    uint8_t* ip = (uint8_t*)&vn_ip.s_addr;
    connect_msg.insert(connect_msg.end(), ip, ip+4);
    connect_msg.push_back(vn_port >> 8);
    connect_msg.push_back(vn_port & 0xff);
    cout<<"VN id "<<vn_id<<" sending connect msg"<<endl;
    send(tcp_fd, connect_msg.data(), connect_msg.size(), 0);

    time_t next_graph = time(NULL) + 15;
    while(1){
        fd_set rf;
        FD_ZERO(&rf);
        FD_SET(udp_fd, &rf);
        int max_fd=udp_fd;
        if(tcp_fd>=0)
        {
            FD_SET(tcp_fd,&rf);
            if(tcp_fd >max_fd)
            {
                max_fd=tcp_fd;
            }
        }
        timeval timeout={1,0};
        select(max_fd+1,&rf, NULL,NULL, &timeout);
        if (tcp_fd >= 0 && FD_ISSET(tcp_fd, &rf)) tcp_lsp();
        if(FD_ISSET(udp_fd, &rf))
        {
            uint8_t buff[4096];
            sockaddr_in src;
            socklen_t src_l=sizeof src;
            int l=recvfrom(udp_fd, buff, sizeof buff,0,(sockaddr*)&src,&src_l);
            if(l>0) udp_lsp(buff, l, src);
        }
        time_t curr=time(NULL);
        if(curr>= next_graph){
            next_graph=curr+10;
            cout<<"VN id "w<<vn_id<<" graph"<<endl;
            for (map<char, Lsp>::iterator it = ls_map.begin(); it!= ls_map.end(); it++){
                cout<<it->first<<":";
                for (int i = 0; i < it->second.nbrs.size(); i++){
                    cout<<" "<<it->second.nbrs[i].id<<"="<<it->second.nbrs[i].d;
                }
                cout<<endl;
            }
        }
        if(first_rcvd && curr>=next_lsp) new_vn_lsp();
    }
}