import socket
import sys

def main():
    file_name = sys.argv[1]

    lines = []

    with open("file_name", "r") as file:
        for line in file:
            if line[0] != '#':
                lines.append(line.split());






if __name__ == "__main__":
    main()