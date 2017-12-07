#!/bin/sh
# test case courtesy of William Schaub (wschaub@steubentech.com)

# remember to use ABSOLUTE PATHS!
rm -f /home/pgbovine/CDE/tests/socket_test/testsocket
./server.py /home/pgbovine/CDE/tests/socket_test/testsocket &
sleep 1
./client.py /home/pgbovine/CDE/tests/socket_test/testsocket <<HEREDOC
In the not so distant future. Way down in deep13
Dr. Forrester and TV's Frank were hatcing an evil scheme.
They hired a temp by the name of mike. Just a reuglar joe they didn't 
like. Their experiment needed a good test case.
So they cokned him on the noggin and they shot him into space.
HEREDOC

