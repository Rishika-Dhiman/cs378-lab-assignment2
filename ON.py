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
    lines = []
    with open(file_name, "r") as file:
        for line in file:
            if is_num(line.split()[0]):
                lines.append([int(x) for x in line.split()])
    
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

graph = get_graph(file_name)

on_ip=""
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("8.8.8.8", 80))
on_ip = s.getsockname()[0]
s.close()


def find_port(ip):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((ip, 0))
        return s.getsockname()[1]

on_port = find_port(on_ip)

print("ON ip: "+ on_ip + "ON port: " + str(on_port))
sock_fd=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock_fd.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)  
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
                
                print("Sent LS msg to VN id: "+ str(chr(i+65)))

    if len(connections) == len(graph) and os.path.getmtime(file_name) != mtime:
        print("Config file changed")
        mtime = os.path.getmtime(file_name)
        graph = []
        graph = get_graph(file_name)
        for i in range(len(graph)):
            connections[i].sendall(link_state_pkt(graph, i, vns))
            print("Sent LS msg to VN id: "+ str(chr(i+65)))

