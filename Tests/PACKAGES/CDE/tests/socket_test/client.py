#!/usr/bin/python
# test case courtesy of William Schaub (wschaub@steubentech.com)

import os, sys
from socket import *
UNIXSOCKET = sys.argv[1]
server = socket(AF_UNIX,SOCK_STREAM)
server.connect(UNIXSOCKET)
while 1:
    data = sys.stdin.readline()
    if not data: break
    server.sendall(data)
server.close()
