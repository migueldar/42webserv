import sys
import os

if __name__ == "__main__":
    data = sys.stdin.read()

    print("Hola! Bienvenido al mundo de Python!\n")

    print(os.environ)

    print(data, end='')
