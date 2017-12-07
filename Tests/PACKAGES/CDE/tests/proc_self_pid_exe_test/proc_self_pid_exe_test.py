# if a program does readlink("/proc/<pid>/exe"), then make sure it
# returns the REAL path to that program rather than the path to the
# dynamic linker

import os

my_pid_str = os.readlink("/proc/self")
res = os.readlink("/proc/%s/exe" % my_pid_str)

print res, len(res)
assert res == "/home/pgbovine/epd-6.2-2-rh5-x86/bin/python"
