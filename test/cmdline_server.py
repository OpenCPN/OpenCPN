#!/usr/bin/env python3

import os
import socket
import sys

SOCKET_PATH = os.path.expanduser("~/.tests/opencpn-ipc")

# remove the socket file if it already exists
try:
    os.unlink(SOCKET_PATH)
except OSError:
    if os.path.exists(SOCKET_PATH):
        raise

server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
server.bind(SOCKET_PATH)
server.listen(1)

connection, client_address = server.accept()
data = connection.recv(1024).decode()
print(data);
sys.exit(0)
