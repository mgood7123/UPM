#!/bin/sh

# remember to use an ABSOLUTE PATH to test redirection
rm -f /home/pgbovine/CDE/tests/hello.fifo
mknod /home/pgbovine/CDE/tests/hello.fifo p
ls ../hello.fifo
