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





if __name__ == "__main__":
    main()