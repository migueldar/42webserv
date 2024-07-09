import sys
import time

if __name__ == "__main__":
    data = sys.stdin.read()

    print("¡Hola! ¡Bienvenido al mundo de Python!\n")

    print(data)

    sys.stdout.flush()

    print("EOF")

    sys.stdout.flush()
