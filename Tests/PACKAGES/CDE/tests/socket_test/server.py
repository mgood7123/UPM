#!/usr/bin/python
# test case courtesy of William Schaub (wschaub@steubentech.com)

import os, sys
from socket import *
UNIXSOCKET = sys.argv[1]
try: 
    os.unlink(UNIXSOCKET)
except:
    pass
server = socket(AF_UNIX,SOCK_STREAM)
server.bind(UNIXSOCKET)
server.listen(1)
client, addr = server.accept()
print "server.py: Accepted connection\n"
while 1:
    data, addr = client.recvfrom(1024)
    if not data: break
    sys.stdout.write(data)
client.close()
