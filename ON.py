import socket
import sys
import struct
import select
import os


def main():
    file_name = sys.argv[1]

    lines = []

    def is_num(s):
        try:
            float(s)
            return True
        except ValueError:
            return False

    with open(file_name, "r") as file:
        for line in file:
            if is_num(line.split()[0]):
                lines.append(line.split());

    print(lines)

    graph=[[0]* (len(lines)+1) for _ in range(len(lines)+1)]
    for i,node in enumerate(lines):
        new_row=[]
        for j,cost in enumerate(node):
            graph[i][j+i+1]=lines[i][j]
            graph[j+i+1][i]=lines[i][j]

    print(graph)
    ip=""
    for x in socket.gethostbyname_ex(socket.gethostname())[2]:
            if not x.startswith("127.")and x != "0.0.0.0":
                ip=x
    tcp_port=5000
    sock_fd=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock_fd.setsockopt(socket.SQL_SOCKET,socket.SQ_REUSEADDR,1)  
    sock_fd.bind((ip,tcp_port))
    sock_fd.listen()
    connections=[]
    ids=[]
    mtime=os.path.getmtime(file_name)
    while True:
        connection,addr=sock_fd.accept()
        connections.append(connection)
        data=connection.recv(6)
        ip_bytes=data[:4]
        udp_port=struct.unpack()
    


if __name__ == "__main__":
    main()