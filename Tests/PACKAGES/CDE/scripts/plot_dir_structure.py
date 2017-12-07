# traverses a directory tree and plots out the resulting directory
# structure to stdout in GraphViz .dot format

# use inode numbers as unique node names (use os.lstat to NOT follow symlinks)

import os, sys, collections

basedir = os.path.realpath(sys.argv[1])

# Key:   node name ('node_' + inode number)
# Value: full path
already_rendered = {}

def get_node_name(path):
  # use lstat to NOT follow symlinks
  return 'node_' + str(os.lstat(path).st_ino)

def get_canonical_name(path):
  return path.split('/')[-1]


print "digraph {"
print 'rankdir="LR"'

for (d, subdirs, files) in os.walk(basedir):
  dirnode = get_node_name(d)
  if dirnode not in already_rendered:
    print dirnode, '[label="%s", shape=box] /* %s */' % (get_canonical_name(d), d)
    already_rendered[dirnode] = d

  for f in files + subdirs:
    p = os.path.join(d, f)
    filenode = get_node_name(p)

    # directory entries get a default solid line
    print '%s->%s' % (dirnode, filenode)

    if os.path.islink(p):
      target = os.path.normpath(os.path.join(d, os.readlink(p)))
      if filenode not in already_rendered:
        print filenode, '[label="%s", shape=diamond] /* %s */' % (get_canonical_name(p), p)
        already_rendered[filenode] = p

      # symlinks get a dashed line!
      print '%s->%s [style=dashed]' % (filenode, get_node_name(target))
    else:
      if filenode not in already_rendered:
        already_rendered[filenode] = p
        if os.path.isfile(p):
          print filenode, '[label="%s", shape=ellipse] /* %s */' % (get_canonical_name(p), p)
        else:
          assert os.path.isdir(p)
          print filenode, '[label="%s", shape=box] /* %s */' % (get_canonical_name(p), p)


# print subgraphs to enforce that all nodes on the same level have the same 'rank'
hash_by_ranks = collections.defaultdict(list)

for (hash, path) in already_rendered.iteritems():
  rank = len(path.split('/'))
  hash_by_ranks[rank].append(hash)

print

maxrank = 1
for (rank, hashes) in hash_by_ranks.iteritems():
  print '  subgraph {'
  print '    rank=same'
  #print '   ', rank
  for h in hashes:
    print '   ', h
  print '  }'
  maxrank = max(maxrank, rank)

#print '->'.join([str(i) for i in range(1, maxrank+1)])

print "}"

