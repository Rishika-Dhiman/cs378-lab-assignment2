import socket
import sys
import struct
import select
import os

def is_num(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def get_graph(file_name):
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
        return graph

def receive_all(connection, n):
    data = b""
    while len(data) < n:
        portion = connection.recv(n - len(data))
        data += portion
    return data

def link_state_pkt(graph, i, vns):
    entries = [(j, graph[i][j]) for j in range(len(graph)) if graph[i][j] >= 0]
    message = struct.pack("!B", len(entries))
    for j, d in entries:
        vn_ip, vn_port = vns[j]
        message += struct.pack("!B4sHh", 65+j, vn_ip, vn_port, d)
    return message


file_name = sys.argv[1]
lines = []

graph = get_graph(file_name)

on_ip=""
for x in socket.gethostbyname_ex(socket.gethostname())[2]:
        if not x.startswith("127.")and x != "0.0.0.0":
            on_ip=x
on_port=5000
sock_fd=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock_fd.setsockopt(socket.SQL_SOCKET,socket.SQ_REUSEADDR,1)  
sock_fd.bind((on_ip,on_port))
sock_fd.listen()
connections=[]
vns=[]
mtime=os.path.getmtime(file_name)
while True:
    ready, _, _ = select.select([sock_fd], [], [], 2.0)
    if ready:  
        connection, _=sock_fd.accept()
        connections.append(connection)
        vn_ip, vn_port = struct.unpack("!4sH", receive_all(connection, 6))
        vns.append((vn_ip, vn_port))
        if len(connections) == len(graph):
            for i in range(len(graph)):
                connections[i].sendall(link_state_pkt(graph, i, vns))

    if len(connections) == len(graph) and os.path.getmtime(file_name) != mtime:
        mtime = os.path.getmtime(file_name)
        graph = get_graph(file_name)
        for i in range(len(graph)):
            connections[i].sendall(link_state_pkt(graph, i, vns))

