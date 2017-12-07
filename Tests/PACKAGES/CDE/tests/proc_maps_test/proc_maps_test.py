# Try to read the contents of /proc/self/maps
# and grab the full path to our python executable.
# Then try to stat that file to get its filesize and print it out.
#
# We want to make sure that running from within cde-package/ gives the
# SAME behavior.

import os

for line in open('/proc/self/maps'):
  tokens = line.split()
  filename = tokens[-1]
  if '/python' in filename:
    print os.stat(filename).st_size
    break
