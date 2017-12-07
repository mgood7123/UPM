# dependencies: bsdiff, md5sum

import os, sys, stat
from subprocess import *

my_root = sys.argv[1]
assert os.path.isdir(my_root)

DEBUG = False

# these files take forever to search thru and don't result in much savings ...
ignores = ['__init__.py', '__init__.pyc']

# Key:   base filename
# Value: list of directories where file is found
system_filenames = {}

basedirs = ['/bin', '/lib', '/lib64', '/usr/bin', '/usr/lib', '/usr/lib64']

for b in basedirs:
  if os.path.isdir(b):
    for (dirname, subdirs, files) in os.walk(b):
      for f in files:
        if f not in system_filenames:
          system_filenames[f] = []
        system_filenames[f].append(dirname)


# candidate pairs for coalescing:
# (full path to file in cde-root/, full path to file in native system # dir)
# Key:   full path to file within cde-root/
# Value: list of candidate system files (full paths)
coalescing_candidates = {}

for (dirname, subdirs, files) in os.walk(my_root):
  for f in files:
    if f in ignores:
      continue

    # first look for exact matches
    if f in system_filenames:
      f_path = os.path.join(dirname, f)
      st = os.lstat(f_path) # don't follow symlinks
      if stat.S_ISREG(st.st_mode):
        for k_dir in system_filenames[f]:
          k_path = os.path.join(k_dir, f)
          k_st = os.lstat(k_path) # don't follow symlinks
          if stat.S_ISREG(k_st.st_mode):
            if f_path not in coalescing_candidates:
              coalescing_candidates[f_path] = []
            coalescing_candidates[f_path].append(k_path)

    # then look for fuzzy searches for libraries
    # by taking all the parts before the first '-' or '.'
    # character and comparing them with contents of
    # system_filenames.  this can pick up on 'variants' of library names
    elif f.startswith('lib'):
      f_path = os.path.join(dirname, f)
      st = os.lstat(f_path) # don't follow symlinks
      if stat.S_ISREG(st.st_mode):
        try: first_dash = f.index('-')
        except ValueError: first_dash = len(f) + 1
        try: first_dot = f.index('.')
        except ValueError: first_dot = len(f) + 1

        i = min(first_dash, first_dot)
        # add 1 to avoid spurious substring matches like
        # libc-2.7.so and libcrypto.so.0.9.8
        base_libname = f[:i+1]

        for k in system_filenames:
          # look for a prefix match
          if k.startswith(base_libname):
            # find all regular files (NOT symlinks)
            for k_dir in system_filenames[k]:
              k_path = os.path.join(k_dir, k)
              k_st = os.lstat(k_path) # don't follow symlinks
              if stat.S_ISREG(k_st.st_mode):
                if f_path not in coalescing_candidates:
                  coalescing_candidates[f_path] = []
                coalescing_candidates[f_path].append(k_path)


if DEBUG:
  print len(coalescing_candidates), 'candidates for coalescing'


total_savings = 0

# TODO: bsdiff does horribly if files are IDENTICAL, so check for
# identicalness first ...

for (x, y_lst) in coalescing_candidates.iteritems():
  best_match = None
  best_match_savings = 0

  if DEBUG: print "Trying:", x

  for y in y_lst:

    (stdout, stderr) = Popen(['md5sum', y, x], stdout=PIPE, stderr=PIPE).communicate()

    lines = stdout.split('\n')
    y_md5 = lines[0].split()[0]

    # sometimes you don't have permissions to read this file, so just
    # move on ...
    try:
      x_md5 = lines[1].split()[0]
    except:
      continue

    if (x_md5 == y_md5):
      if DEBUG: print 'EXACT MATCH!', y
      best_match = y
      pkg_st = os.stat(x)
      best_match_savings = pkg_st.st_size
      break # break out of this loop altogether


    if os.path.exists('/tmp/cur.patch'):
      os.remove('/tmp/cur.patch')
    # pass in the system's version of the file as the first arg
    (stdout, stderr) = Popen(['./bsdiff', y, x, '/tmp/cur.patch'], stdout=PIPE, stderr=PIPE).communicate()

    pkg_st = os.stat(x)
    try:
      patch_st = os.stat('/tmp/cur.patch')
    except:
      print >> sys.stderr, "Error in bsdiff:", y, x
      continue # sometimes bsdiff fails

    savings = pkg_st.st_size - patch_st.st_size
    if savings < 0:
      if DEBUG: print "WARNING:", y
      pass
    else:
      if savings > best_match_savings:
        best_match_savings = savings
        best_match = y
        if DEBUG: print "  better:", y

  if DEBUG: print "Best match:", best_match
  if DEBUG: print "  bytes saved:", best_match_savings
  if DEBUG: print "---"
  total_savings += best_match_savings


if DEBUG:
  print "Total saved bytes:", total_savings
else:
  print total_savings

