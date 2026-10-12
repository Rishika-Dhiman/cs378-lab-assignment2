import socket
import sys

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


if __name__ == "__main__":
    main()