# Script that interactively guides the user to completing a package

# TODO: fix limitation with rsync NOT properly handling symlinks to absolute paths

# TODO: refactor to use subprocess.call(['okapi', ...]) rather than
# rsync, since okapi gracefully handles symlinks.  Use os.walk() to walk
# through a directory structure and use okapi to copy all constituents
# into the package.

# TODO: the NUMBER of sub-directories contained in a directory (within
# the package) might be a proxy for its "importance" and could be used
# for ranking


import os, sys, math
from collections import defaultdict
from subprocess import call

CDE_ROOT = '/cde-root'

ignored_dirs_set = set()

# returns a dict mapping extension name to frequency of occurrence
def get_extensions_histogram(filelst):
  ret = defaultdict(int)
  for f in filelst:
    # special handling for '.so' to find files like 'libXcomposite.so.1.0.0'
    if '.so.' in f:
      ret['.so'] += 1
    else:
      ret[os.path.splitext(f)[1]] += 1
  return ret

# returns True iff d1 is a child directory of d2
def is_child_dir(d1, d2):
  return d1.startswith(d2) and d1[len(d2)] == '/'


class DirEntry:
  pass


# returns cumulative number of files and cumulative number of
# sub-directories in the current directory d
def get_cum_num_files_subdirs(dirname):
  num_files = 0
  num_subdirs = 0
  for (dn, subdirs, files) in os.walk(dirname):
    num_files += len(files)
    num_subdirs += len(subdirs)
  return (num_files, num_subdirs)


# log_fn should be the filename of a log produced by running 'cde -c'
def parse_log(log_fn):
  files = []
  for line in open(log_fn):
    line = line.strip()

    # VERY IMPORTANT - get the REAL PATH after resolving all symlinks
    rp = os.path.realpath(line)

    # experiment with only keeping the FIRST occurrence of each rp
    #if rp in files: continue

    files.append(rp)

  dirs_set = set(f for f in files if os.path.isdir(f))
  files = [e for e in files if e not in dirs_set] # filter out dirs

  dirnames = [os.path.dirname(f) for f in files]

  # Key: dirname
  # Value: list of indices of where it appears in dirnames
  appearance_indices = defaultdict(list)

  max_index = len(dirnames) - 1

  for (i, d) in enumerate(dirnames):
    # append this normalized index to all parent directories as well:
    cur = d
    while cur != '/':
      appearance_indices[cur].append(float(i) / max_index)
      cur = os.path.dirname(cur)


  # calculate mean
  dirnames_and_scores = [(k, float(sum(v)) / len(v)) for (k,v) in appearance_indices.iteritems()]
  # calculate median
  #dirnames_and_scores = [(k, sorted(v)[len(v)/2]) for (k,v) in appearance_indices.iteritems()]

  dirnames_and_scores.sort(key = lambda e:e[1], reverse=True)

  return dict(dirnames_and_scores)


def run_cde2(package_dir, logfile):

  log_scores = parse_log(logfile)

  for e in sorted(log_scores.keys()):
    print e, log_scores[e]


  while True:
    dat = []

    for (dirname, subdirs, files) in os.walk(package_dir):
      if CDE_ROOT not in dirname: continue
      system_dir = dirname[dirname.find(CDE_ROOT) + len(CDE_ROOT):]
      if not system_dir: continue

      if not os.path.isdir(system_dir):
        print "WARNING:", system_dir, "is in package but not on system."

      if system_dir in ignored_dirs_set: continue

      d = DirEntry()

      d.name = dirname
      d.system_dirname = system_dir

      d.nesting_level = d.system_dirname.count('/')

      try:
        d.log_score = log_scores[d.system_dirname]
      except KeyError:
        d.log_score = 0

      d.cum_num_files, d.cum_num_subdirs = get_cum_num_files_subdirs(d.name)

      # system_dirname can be HUGE if it's a top-level directory, so SKIP analyzing it if it takes forever
      try:
        print 'Calling get_cum_num_files_subdirs("' + d.system_dirname + '")'
        d.cum_num_system_files, d.cum_num_system_subdirs =  get_cum_num_files_subdirs(d.system_dirname)
      except KeyboardInterrupt:
        ignored_dirs_set.add(d.system_dirname)
        continue

      # sum of squares to calculate 'euclidian distance'
      d.cum_score = 0

      # file coverage:

      #try: d.cum_score += pow(float(d.cum_num_files) / float(d.cum_num_system_files), 2)
      #except ZeroDivisionError: pass
      # add by 1 to penalize small values:
      d.cum_score += pow(float(d.cum_num_files) / float(d.cum_num_system_files + 1), 2)

      # sub-directory coverage:
      # TODO: nix this for now

      #try: d.cum_score += pow(float(d.cum_num_subdirs) / float(d.cum_num_system_subdirs), 2)
      #except ZeroDivisionError: pass
      # add by 1 to penalize small values:
      #d.cum_score += pow(float(d.cum_num_subdirs) / float(d.cum_num_system_subdirs + 1), 2)

      # mean normalized occurrence order:
      d.cum_score += pow(d.log_score, 2)

      dat.append(d)

    dat.sort(key = lambda d: d.cum_score, reverse=True)

    # filter all completely-empty and completely-full directories
    dat = [d for d in dat if d.cum_num_files > 0 and d.cum_num_files < d.cum_num_system_files]

    # optional filter ... filter all sub-directories with LOWER scores
    # than their parents ... wow, this seems to be REALLY useful :)
    '''
    filtered_dat = []
    for d in dat:
      reject = False
      for fd in filtered_dat:
        if is_child_dir(d.system_dirname, fd.system_dirname):
          reject = True
          break

      if reject:
        #print 'REJECTED:', d.system_dirname, 'due to', fd.system_dirname
        pass
      else:
        filtered_dat.append(d)

    dat = filtered_dat
    '''

    for (i, d) in enumerate(dat):
      #if i >= 20: break
      print i + 1, ')', d.system_dirname, round(d.cum_score, 3), \
            '- %d/%d files,' % (d.cum_num_files, d.cum_num_system_files), \
            '%d/%d subdirs' % (d.cum_num_subdirs, d.cum_num_system_subdirs), \
            'lscore:', round(d.log_score, 3)

    print
    print "Choose sub-directory to copy into package ('q' to quit):",
    choice = raw_input()
    if choice == 'q':
      return
    else:
      choice = int(choice) - 1 # so we can be one-indexed for user-friendliness
    selected = dat[choice]

    # remember to put a trailing '/' to get rsync to work properly
    #
    # TODO: a problem with rsync is that if directories contain symlinks
    # to absolute paths, the symlinks won't be properly re-written to
    # point to the proper versions within cde-package/cde-root/
    #
    # see the code for create_symlink_in_cde_root() in cde.c for subtle
    # details about how to copy symlinks into cde-package/cde-root/
    #
    # also look into 'man rsync' for these options, which might help:
    #
    # -l, --links                 copy symlinks as symlinks
    # -L, --copy-links            transform symlink into referent file/dir
    #     --copy-unsafe-links     only "unsafe" symlinks are transformed
    #     --safe-links            ignore symlinks that point outside the tree
    # -k, --copy-dirlinks         transform symlink to dir into referent dir
    # -K, --keep-dirlinks         treat symlinked dir on receiver as dir
    #
    args = ['rsync', '-a', selected.system_dirname + '/', selected.name + '/']
    print args
    ret = call(args)
    assert ret == 0


# returns True if d.name either contains NO FILES or contains the complete set
# of files in its corresponding d.system_dirname.
# (this runs fine even if d.system_dirname contains a TON of files)
def package_dir_is_full(d):
  package_num_files = 0
  for (dn, subdirs, files) in os.walk(d.name):
    package_num_files += len(files)

  # empty :)
  if package_num_files == 0:
    return True

  system_num_files = 0
  for (dn, subdirs, files) in os.walk(d.system_dirname):
    system_num_files += len(files)
    # this early return is VITAL, since d.system_dirname could contain a
    # GIGANTIC number of files, and this function will run forever if not for
    # early termination :)
    if system_num_files > package_num_files:
      return False

  # full :)
  return True



def run_simple_package_completer(package_dir):
  assert os.path.isdir(package_dir + CDE_ROOT)

  while True:
    dat = []

    for (dirname, subdirs, files) in os.walk(package_dir):
      if CDE_ROOT not in dirname: continue
      system_dir = dirname[dirname.find(CDE_ROOT) + len(CDE_ROOT):]
      if not system_dir: continue

      if not os.path.isdir(system_dir):
        print "WARNING:", system_dir, "is in package but not on system."

      d = DirEntry()

      d.name = dirname
      d.system_dirname = system_dir

      d.nesting_level = d.system_dirname.count('/')

      if not package_dir_is_full(d):
          dat.append(d)


    for (i, d) in enumerate(dat):
      print i + 1, ')\t' + d.system_dirname

    print
    print "Choose sub-directory to copy into package ('q' to quit):",
    choice = raw_input()
    if choice == 'q':
      return
    else:
      choice = int(choice) - 1 # so we can be one-indexed for user-friendliness
    selected = dat[choice]

    # remember to put a trailing '/' to get rsync to work properly
    #
    # TODO: a problem with rsync is that if directories contain symlinks
    # to absolute paths, the symlinks won't be properly re-written to
    # point to the proper versions within cde-package/cde-root/
    #
    # see the code for create_symlink_in_cde_root() in cde.c for subtle
    # details about how to copy symlinks into cde-package/cde-root/
    #
    # also look into 'man rsync' for these options, which might help:
    #
    # -l, --links                 copy symlinks as symlinks
    # -L, --copy-links            transform symlink into referent file/dir
    #     --copy-unsafe-links     only "unsafe" symlinks are transformed
    #     --safe-links            ignore symlinks that point outside the tree
    # -k, --copy-dirlinks         transform symlink to dir into referent dir
    # -K, --keep-dirlinks         treat symlinked dir on receiver as dir
    #
    args = ['rsync', '-a', selected.system_dirname + '/', selected.name + '/']
    print args
    ret = call(args)
    assert ret == 0



if __name__ == "__main__":
  run_simple_package_completer(sys.argv[1])
  #run_cde2('cde-package/', 'cde-copied-files.log')
