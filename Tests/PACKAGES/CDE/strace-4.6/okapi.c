/*

okapi (pronounced "oh-copy") is a robust file copying utility for Linux
that gracefully handles the utter grossness of symlinks and
sub-directories.

Created by Philip Guo on 2011-08-02

okapi is currently included as a library within the CDE project.

To make a stand-alone okapi binary, compile with -DOKAPI_STANDALONE, e.g.,:

To invoke, run:

  okapi <absolute_path> <src_prefix> <dst_prefix>

okapi will copy $src_prefix/$absolute_path into $dst_prefix/$absolute_path,
creating all necessary layers of symlinks and sub-directories along the way.
$src_prefix may be "".


TODOs:
  - make explicit to the user that hard links are being used

*/


/*

CDE: Code, Data, and Environment packaging for Linux
http://www.stanford.edu/~pgbovine/cde.html
Philip Guo

CDE is currently licensed under GPL v3:

  Copyright (c) 2010 Philip Guo <pg@cs.stanford.edu>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

*/

#include "okapi.h"

char OKAPI_VERBOSE = 1; // print out warning messages?


// TODO: eliminate this hack if it results in a compile-time error
#include "config.h" // to get I386 definition
#if defined (I386)
// This forces gcc to use an older version of realpath from glibc 2.0,
// to maximize backwards compatibility
// See: http://www.trevorpounds.com/blog/?p=103
__asm__(".symver realpath,realpath@GLIBC_2.0");
#endif


#include <stdarg.h>
extern char* format(const char *format, ...);

static struct path* new_path(char is_abspath);

// note that realpath_strdup and realpath_nofollow seem to do funny
// things to arguments that are directories (okay for regular files,
// though)

// calls realpath and mallocs a new result string
// Pre-req: filename must actually exist on the filesystem!
//
// mallocs a new string
char* realpath_strdup(char* filename) {
  char path[MAXPATHLEN];
  path[0] = '\0';
  char* ret = realpath(filename, path);
  assert(ret); // the target path must actually exist!

  assert(path[0] == '/'); // must be an absolute path
  return strdup(path);
}

// mallocs a new string
char* readlink_strdup(char* filename) {
  char path[MAXPATHLEN];
  path[0] = '\0';
  int len = readlink(filename, path, sizeof path);
  assert(path[0] != '\0');

  assert(len >= 0);
  path[len] = '\0'; // wow, readlink doesn't put the cap on the end!
  return strdup(path);
}


// representing and manipulating path components
// (code courtesy of the Goanna project,
//  http://tcos.org/project-goanna.html)

static void empty_path(struct path *path) {
  int pos = 0;
  path->depth = 0;
  if (path->stack) {
    while (path->stack[pos]) {
      free(path->stack[pos]);
      path->stack[pos] = NULL;
      pos++;
    }
  }
}


// pop the last element of path
void path_pop(struct path* p) {
  if (p->depth == 0) {
    return;
  }

  free(p->stack[p->depth-1]);
  p->stack[p->depth-1] = NULL;
  p->depth--;
}

// paths are indexed starting at 1
char* get_path_component(struct path* p, int ind) {
  assert(ind <= p->depth);
  char* ret = p->stack[ind - 1]->str;
  assert(ret);
  return ret;
}

static struct path* new_path_internal(char* path, char is_abspath) {
  int stackleft;

  path = strdup(path); // so that we don't clobber the original
  char* path_dup_base = path; // for free()

  struct path* base = new_path(is_abspath);

  if (is_abspath) {
    empty_path(base);
    path++;
  }

  stackleft = base->stacksize - base->depth - 1;

  do {
    char *p;
    while (stackleft <= 1) {
      base->stacksize *= 2;
      stackleft = base->stacksize / 2;
      base->stacksize++;
      stackleft++;
      base->stack =
        (struct namecomp **)realloc(base->stack, base->stacksize * sizeof(struct namecomp*));
      assert(base->stack);
    }

    // Skip multiple adjoining slashes
    while (*path == '/') {
      path++;
    }

    p = strchr(path, '/');
    // put a temporary stop-gap ... uhhh, this assumes path isn't read-only
    if (p) {
      *p = '\0';
    }

    if (path[0] == '\0') {
      base->stack[base->depth] = NULL;
      // We are at the end (or root), do nothing.
    }
    else if (!strcmp(path, ".")) {
      base->stack[base->depth] = NULL;
      // This doesn't change anything.
    }
    else if (!strcmp(path, "..")) {
      if (base->depth > 0) {
        free(base->stack[--base->depth]);
        base->stack[base->depth] = NULL;
        stackleft++;
      }
    }
    else {
      base->stack[base->depth] =
        (struct namecomp *)malloc(sizeof(struct namecomp) + strlen(path) + 1);
      assert(base->stack[base->depth]);
      strcpy(base->stack[base->depth]->str, path);
      base->stack[base->depth]->len = strlen(path);
      base->depth++;
      base->stack[base->depth] = NULL;
      stackleft--;
    }

    // Put it back the way it was
    if (p) {
      *p++ = '/';
    }
    path = p;
  } while (path);

  free(path_dup_base);
  return base;
}


// creates a canonicalized path object by removing all instances of '.'
// and '..' from an absolute path
//
// mallocs a new path object, must free using delete_path(),
// NOT using ordinary free()
struct path* new_path_from_abspath(char* path) {
  assert(IS_ABSPATH(path));
  return new_path_internal(path, 1);
}

// creates a path object from a relative path, resolving it relative to
// base, which must be an absolute path
//
// mallocs a new path object, must free using delete_path(),
// NOT using ordinary free()
struct path* new_path_from_relpath(char* relpath, char* base) {
  assert(!IS_ABSPATH(relpath));
  assert(IS_ABSPATH(base));

  char* tmp = format("%s/%s", base, relpath);
  struct path* ret = new_path_from_abspath(tmp);

  free(tmp);
  return ret;
}

// canonicalizes an absolute path, mallocs a new string
char* canonicalize_abspath(char* abspath) {
  struct path* p = new_path_from_abspath(abspath);
  char* ret = path2str(p, 0);
  delete_path(p);
  return ret;
}

// canonicalizes a relative path with respect to base, mallocs a new string
static char* canonicalize_relpath(char* relpath, char* base) {
  struct path* p = new_path_from_relpath(relpath, base);
  char* ret = path2str(p, 0);
  delete_path(p);
  return ret;
}

char* canonicalize_path(char* path, char* relpath_base) {
  if (IS_ABSPATH(path)) {
    return canonicalize_abspath(path);
  }
  else {
    return canonicalize_relpath(path, relpath_base);
  }
}


static struct path* new_path(char is_abspath) {
  struct path* ret = (struct path *)malloc(sizeof(struct path));
  assert(ret);

  ret->stacksize = 1;
  ret->is_abspath = is_abspath;
  ret->depth = 0;
  ret->stack = (struct namecomp **)malloc(sizeof(struct namecomp *));
  assert(ret->stack);
  ret->stack[0] = NULL;
  return ret;
}

void delete_path(struct path *path) {
  assert(path);
  if (path->stack) {
    int pos = 0;
    while (path->stack[pos]) {
      free(path->stack[pos]);
      path->stack[pos] = NULL;
      pos++;
    }
    free(path->stack);
  }
  free(path);
}


// mallocs a new string and populates it with up to
// 'depth' path components (if depth is 0, uses entire path)
char* path2str(struct path* path, int depth) {
  int i;
  int destlen = 2; // at least have room for '/' and null terminator ('\0')

  // simply use path->depth if depth is out of range
  if (depth <= 0 || depth > path->depth) {
    depth = path->depth;
  }

  for (i = 0; i < depth; i++) {
    destlen += path->stack[i]->len + 1;
  }

  char* dest = (char *)malloc(destlen);

  char* ret = dest;

  // print a leading '/' for absolute paths
  if (path->is_abspath) {
    *dest++ = '/';
    destlen--;
  }

  for (i = 0; i < depth; i++) {
    assert(destlen >= path->stack[i]->len + 1);

    memcpy(dest, path->stack[i]->str, path->stack[i]->len);
    dest += path->stack[i]->len;
    destlen -= path->stack[i]->len;

    if (i < depth - 1) { // do we have a successor?
      assert(destlen >= 2);
      *dest++ = '/';
      destlen--;
    }
  }

  *dest = '\0';

  return ret;
}


// return 1 iff the absolute path of filename is within target_dir
// (for relative paths, calculate their locations relative to relative_path_basedir)
int file_is_within_dir(char* filename, char* target_dir, char* relative_path_basedir) {
  char* fake_cano_dir = canonicalize_abspath(target_dir);
  // very subtle --- if fake_cano_dir isn't simply '/' (root directory),
  // tack on a '/' to the end of fake_cano_dir, so that we
  // don't get misled by substring comparisons.  canonicalize_abspath
  // does NOT put on a trailing '/' for directories.
  //
  // e.g., "/home/pgbovine/hello.txt" is NOT within the
  // "/home/pgbovine/hello" directory, so we need to tack on an extra
  // '/' to the end of cano_dir to make it "/home/pgbovine/hello/" in
  // order to avoid a false match
  int fake_cano_dir_len = strlen(fake_cano_dir);

  char* cano_dir = NULL;
  if (fake_cano_dir_len > 1) {
    cano_dir = (char*)malloc(fake_cano_dir_len + 2);
    strcpy(cano_dir, fake_cano_dir);
    cano_dir[fake_cano_dir_len] = '/';
    cano_dir[fake_cano_dir_len + 1] = '\0';
  }
  else {
    cano_dir = strdup(fake_cano_dir);
  }
  assert(cano_dir);
  int cano_dir_len = strlen(cano_dir);

  // do a similar hack by tacking on a '/' to the end of filename so
  // that we can find exact directory matches like:
  // "/home/pgbovine/hello" should be contained within itself ... we're
  // then comparing:
  // "/home/pgbovine/hello/" == "/home/pgbovine/hello/"
  //
  // but notice that /home/pgbovine/hello.txt is NOT within
  // /home/pgbovine/hello/ ...
  // "/home/pgbovine/hello.txt/" != "/home/pgbovine/hello/"
  //
  char* fake_cano_filename = canonicalize_path(filename, relative_path_basedir);
  int fake_cano_filename_len = strlen(fake_cano_filename);

  char* cano_filename = NULL;
  if (fake_cano_filename_len > 1) {
    cano_filename = (char*)malloc(fake_cano_filename_len + 2);
    strcpy(cano_filename, fake_cano_filename);
    cano_filename[fake_cano_filename_len] = '/';
    cano_filename[fake_cano_filename_len + 1] = '\0';
  }
  else {
    cano_filename = strdup(fake_cano_filename);
  }
  assert(cano_filename);
  int cano_filename_len = strlen(cano_filename);


  // now that they are canonicalized, we can do a simple substring comparison:
  char is_within_pwd = 0;
  if ((cano_dir_len <= cano_filename_len) &&
      (strncmp(cano_dir, cano_filename, cano_dir_len) == 0)) {
    is_within_pwd = 1;
  }

  free(fake_cano_dir);
  free(cano_dir);
  free(fake_cano_filename);
  free(cano_filename);

  return is_within_pwd;
}


// useful utility function from ccache codebase
// http://ccache.samba.org/
/* Construct a string according to a format. Caller frees. */
char* format(const char *format, ...) {
  va_list ap;
  char *ptr = NULL;

  va_start(ap, format);
  vasprintf(&ptr, format, ap);
  va_end(ap);

  assert(*ptr);
  return ptr;
}


// emulate the functionality of:
// "cp $src_prefix/$filename_abspath $dst_prefix/$filename_abspath",
// creating all constituent sub-directories in the process.
//
// ($src_prefix shouldn't be NULL; instead, use "" for an empty prefix.)
//
// if $filename_abspath is a symlink, then copy both it AND its target into $dst_prefix/
void create_mirror_file(char* filename_abspath, char* src_prefix, char* dst_prefix) {
  assert(IS_ABSPATH(filename_abspath));
  assert(IS_ABSPATH(dst_prefix));

  char* src_path = format("%s%s", src_prefix, filename_abspath);
  char* dst_path = format("%s%s", dst_prefix, filename_abspath);
  assert(IS_ABSPATH(src_path));
  assert(IS_ABSPATH(dst_path));


  // this will NOT follow the symlink ...
  struct stat src_path_stat;
  if (lstat(src_path, &src_path_stat)) {
    // be failure-oblivious here
    if (OKAPI_VERBOSE) {
      fprintf(stderr, "WARNING: cannot mirror '%s' since it does not exist\n", src_path);
    }
    goto done;
  }

  char is_symlink = S_ISLNK(src_path_stat.st_mode);

  if (is_symlink) {
    // 'stat' will follow the symlink ...
    if (stat(src_path, &src_path_stat)) {
      // be failure-oblivious here
      if (OKAPI_VERBOSE) {
        fprintf(stderr, "WARNING: the symlink '%s' has a non-existent target\n", src_path);
      }

      // DON'T PUNT HERE ... simply issue a warning and keep executing ...
      //goto done;
    }
  }

  // by now, src_path_stat contains the info for the actual target file,
  // NOT a symlink to it

  if (S_ISREG(src_path_stat.st_mode)) { // regular file or symlink to regular file
    // lazy optimization to avoid redundant copies ...
    struct stat dst_path_stat;
    if (stat(dst_path, &dst_path_stat) == 0) {
      // if the destination file exists and is newer than the original
      // filename, then don't do anything!
      if (dst_path_stat.st_mtime >= src_path_stat.st_mtime) {
        goto done;
      }
    }
  }

  // finally, 'copy' src_path over to dst_path

  // if it's a symlink, copy both it and its target
  if (is_symlink) {
    create_mirror_symlink_and_target(filename_abspath, src_prefix, dst_prefix);
  }
  else {
    if (S_ISREG(src_path_stat.st_mode)) { // regular file
      // create all the directories leading up to it, to make sure file
      // copying/hard-linking will later succeed
      create_mirror_dirs(filename_abspath, src_prefix, dst_prefix, 1);

      // 1.) try a hard link for efficiency
      // 2.) if that fails, then do a straight-up copy,
      //     but do NOT follow symlinks
      //
      // EEXIST means the file already exists, which isn't
      // really a hard link failure ...
      if ((link(src_path, dst_path) != 0) && (errno != EEXIST)) {
        copy_file(src_path, dst_path, 0);
      }
    }
    else if (S_ISDIR(src_path_stat.st_mode)) { // directory or symlink to directory
      create_mirror_dirs(filename_abspath, src_prefix, dst_prefix, 0);
    }
  }

done:
  free(src_path);
  free(dst_path);
}


// emulate the functionality of "mkdir -p $dst_prefix/$original_abspath",
// creating all the corresponding 'mirror' directories within
// $dst_prefix, but making sure to create directory symlinks
// when necessary if any path component of $src_prefix/$original_abspath
// is a symlink.
//
// ($src_prefix shouldn't be NULL; instead, use "" for an empty prefix.)
//
// if pop_one is non-zero, then pop last element of original_abspath
// before doing the "mkdir -p".
void create_mirror_dirs(char* original_abspath, char* src_prefix, char* dst_prefix, int pop_one) {
  assert(IS_ABSPATH(original_abspath));
  assert(IS_ABSPATH(dst_prefix));

  struct path* p = new_path_from_abspath(original_abspath);

  if (pop_one) {
    path_pop(p); // e.g., ignore filename portion to leave just the dirname
  }

  int i;
  for (i = 1; i <= p->depth; i++) {
    char* dn = path2str(p, i);
    assert(IS_ABSPATH(dn));
    char* dst_dirname = format("%s%s", dst_prefix, dn);
    assert(IS_ABSPATH(dst_dirname));

    // only do this if dst_dirname doesn't already exist
    // (to prevent possible infinite loops)
    struct stat already_exists_stat;
    if (lstat(dst_dirname, &already_exists_stat) != 0) {

      // check for the existence of $src_prefix/$dn:
      char* src_dirname = format("%s%s", src_prefix, dn);
      assert(IS_ABSPATH(src_dirname));

      struct stat src_dn_stat;
      if (lstat(src_dirname, &src_dn_stat) == 0) { // this does NOT follow the symlink
        char is_symlink = S_ISLNK(src_dn_stat.st_mode);
        if (is_symlink) {
          create_mirror_symlink_and_target(dn, src_prefix, dst_prefix);
        }
        else {
          assert(S_ISDIR(src_dn_stat.st_mode));
          mkdir(dst_dirname, 0777);
        }
      }

      free(src_dirname);
    }

    free(dst_dirname);
    free(dn);
  }
  delete_path(p);
}


// copy a symlink from $src_prefix/$filename_abspath into $dst_prefix/$filename_abspath,
// and also copy over the symlink's target into $dst_prefix/ (tricky!!!)
//
// if $src_prefix/$filename_abspath is a symlink to an ABSOLUTE PATH,
// then make $dst_prefix/$filename_abspath be a symlink to a RELATIVE
// path, calculated relative to $dst_prefix.
//
// recursively handle cases where there are symlinks to other symlinks,
// so that we need to create multiple levels of symlinks!
//
// Pre: $src_prefix/$filename_abspath is actually a symlink
void create_mirror_symlink_and_target(char* filename_abspath, char* src_prefix, char* dst_prefix) {
  assert(IS_ABSPATH(filename_abspath));
  assert(IS_ABSPATH(dst_prefix));

  int src_prefix_len = strlen(src_prefix);

  char* src_path = format("%s%s", src_prefix, filename_abspath);
  assert(IS_ABSPATH(src_path));

  // Precondition check ...
  struct stat src_path_stat;
  if (lstat(src_path, &src_path_stat)) { // this will NOT follow the symlink ...
    fprintf(stderr, "FATAL ERROR: lstat('%s') failed\n", src_path);
    exit(1);
  }
  assert(S_ISLNK(src_path_stat.st_mode));


  // target file must exist, so let's resolve its full path
  char* orig_symlink_target = readlink_strdup(src_path);

  char* src_path_copy = strdup(src_path); // dirname() destroys its arg
  char* dir = dirname(src_path_copy);
  char* dir_realpath = realpath_strdup(dir);
  free(src_path_copy);

  char* dst_symlink_path = format("%s%s", dst_prefix, filename_abspath);

  // make sure parent directories exist
  create_mirror_dirs(filename_abspath, src_prefix, dst_prefix, 1);

  char* symlink_target_abspath = NULL;

  // ugh, remember that symlinks can point to both absolute AND
  // relative paths ...
  if (IS_ABSPATH(orig_symlink_target)) {
    symlink_target_abspath = strdup(orig_symlink_target);

    // this is sort of tricky.  we need to insert in a bunch of ../
    // to bring the directory BACK UP to $dst_prefix, and then we need
    // to insert in the original absolute path, in order to make the
    // symlink in the CDE package a RELATIVE path starting from
    // the $dst_prefix base directory

    assert((strlen(dir_realpath) >= src_prefix_len) &&
           (strncmp(dir_realpath, src_prefix, src_prefix_len) == 0));

    char* dir_realpath_suffix = dir_realpath + src_prefix_len;
    assert(IS_ABSPATH(dir_realpath_suffix));

    char relative_symlink_target[MAXPATHLEN];

    // ALWAYS start with this distinctive marker, which makes this path
    // into a relative path ...
    strcpy(relative_symlink_target, "./");

    struct path* p = new_path_from_abspath(dir_realpath_suffix);
    int i;
    for (i = 0; i < p->depth; i++) {
      strcat(relative_symlink_target, "../");
    }
    delete_path(p);

    assert((strlen(orig_symlink_target) >= src_prefix_len) &&
           (strncmp(orig_symlink_target, src_prefix, src_prefix_len) == 0));

    char* orig_symlink_target_suffix = orig_symlink_target + src_prefix_len;
    assert(IS_ABSPATH(orig_symlink_target_suffix));
    strcat(relative_symlink_target, orig_symlink_target_suffix);

    // make sure the path starts with "./" and contains "//":
    assert(strncmp(relative_symlink_target, "./", 2) == 0);
    assert(strstr(relative_symlink_target, "//"));

    // EEXIST means the file already exists, which isn't really a symlink failure ...
    if (symlink(relative_symlink_target, dst_symlink_path) != 0 && (errno != EEXIST)) {
      if (OKAPI_VERBOSE) {
        fprintf(stderr, "WARNING: symlink('%s', '%s') failed\n", relative_symlink_target, dst_symlink_path);
      }
    }
  }
  else {
    symlink_target_abspath = format("%s/%s", dir_realpath, orig_symlink_target);
    // EEXIST means the file already exists, which isn't really a symlink failure ...
    if (symlink(orig_symlink_target, dst_symlink_path) != 0 && (errno != EEXIST)) {
      if (OKAPI_VERBOSE) {
        fprintf(stderr, "WARNING: symlink('%s', '%s') failed\n", orig_symlink_target, dst_symlink_path);
      }
    }
  }

  assert(symlink_target_abspath);
  assert(IS_ABSPATH(symlink_target_abspath));

  free(dir_realpath);
  free(dst_symlink_path);
  free(orig_symlink_target);

  // symlink_target_abspath should always start with src_prefix
  // (if src_prefix is "", then that's trivially true).

  assert((strlen(symlink_target_abspath) >= src_prefix_len) &&
         (strncmp(symlink_target_abspath, src_prefix, src_prefix_len) == 0));

  // $symlink_target_abspath == $src_prefix + $symlink_target_abspath_suffix
  char* symlink_target_abspath_suffix = symlink_target_abspath + src_prefix_len;
  assert(IS_ABSPATH(symlink_target_abspath_suffix));


  struct stat symlink_target_stat;
  if (lstat(symlink_target_abspath, &symlink_target_stat)) { // lstat does NOT follow symlinks
    if (OKAPI_VERBOSE) {
      fprintf(stderr, "WARNING: '%s' (symlink target) does not exist\n", symlink_target_abspath);
    }
    goto done;
  }

  if (S_ISLNK(symlink_target_stat.st_mode)) {
    /* this is super nasty ... we need to handle multiple levels of
       symlinks ... yes, symlinks to symlinks!

      some programs like java are really picky about the EXACT directory
      structure being replicated within cde-package.  e.g., java will refuse
      to start unless the directory structure is perfectly mimicked (since it
      uses its true path to load start-up libraries).  this means that CDE
      Needs to be able to potentially traverse through multiple levels of
      symlinks and faithfully recreate them within cde-package.

      For example, on chongzi (Fedora Core 9):

      /usr/bin/java is a symlink to /etc/alternatives/java

      but /etc/alternatives/java is itself a symlink to /usr/lib/jvm/jre-1.6.0-openjdk/bin/java

      this example involves 2 levels of symlinks, and java requires that the
      TRUE binary to be found here in the package in order to run properly:

        /usr/lib/jvm/jre-1.6.0-openjdk/bin/java

    */
    // krazy rekursive kall!!!
    create_mirror_symlink_and_target(symlink_target_abspath_suffix, src_prefix, dst_prefix);
  }
  else {
    // ok, let's get the absolute path without any '..' or '.' funniness
    // MUST DO IT IN THIS ORDER, OR IT WILL EXHIBIT SUBTLE BUGS!!!
    char* symlink_dst_original_path = canonicalize_abspath(symlink_target_abspath);

    assert((strlen(symlink_dst_original_path) >= src_prefix_len) &&
           (strncmp(symlink_dst_original_path, src_prefix, src_prefix_len) == 0));

    char* symlink_dst_original_path_suffix = symlink_dst_original_path + src_prefix_len;
    assert(IS_ABSPATH(symlink_dst_original_path_suffix));

    char* symlink_dst_abspath = format("%s%s", dst_prefix, symlink_dst_original_path_suffix);

    if (S_ISREG(symlink_target_stat.st_mode)) {
      // base case, just hard link or copy symlink_target_abspath into symlink_dst_abspath

      // ugh, this is getting really really gross, mkdir all dirs stated in
      // symlink_dst_abspath if they don't yet exist
      create_mirror_dirs(symlink_dst_original_path_suffix, src_prefix, dst_prefix, 1);

      if ((link(symlink_target_abspath, symlink_dst_abspath) != 0) && (errno != EEXIST)) {
        copy_file(symlink_target_abspath, symlink_dst_abspath, 0);
      }
    }
    else if (S_ISDIR(symlink_target_stat.st_mode)) { // symlink to directory
      // make sure the target directory actually exists
      create_mirror_dirs(symlink_dst_original_path_suffix, src_prefix, dst_prefix, 0);
    }
    else {
      if (OKAPI_VERBOSE) {
        fprintf(stderr, "WARNING: create_mirror_symlink_and_target('%s') has unknown target file type\n",
                filename_abspath);
      }
    }

    free(symlink_dst_abspath);
    free(symlink_dst_original_path);
  }

done:
  free(symlink_target_abspath);
  free(src_path);
}


// do a straight-up binary copy of $src_filename into $dst_filename.
// if perms == 0, then dst_filename will acquire the SAME file
// permissions as src_filename.  Otherwise, dst_filename will have
// its permissions set to perms.
// note that this WILL follow symlinks
void copy_file(char* src_filename, char* dst_filename, int perms) {
  int inF;
  int outF;
  int bytes;
  char buf[4096]; // TODO: consider using BUFSIZ if it works better

  //printf("COPY %s %s\n", src_filename, dst_filename);

  // do a full-on copy

  if (perms == 0) {
    // match perms of src_filename
    struct stat src_stat;
    if (lstat(src_filename, &src_stat) == 0) {
      perms = src_stat.st_mode;
    }
  }

  // default to most liberal permissions if stat failed ...
  if (perms == 0) {
    perms = 0777;
  }


  inF = open(src_filename, O_RDONLY); // note that we might not have permission to open src_filename
  if ((outF = open(dst_filename, O_WRONLY | O_CREAT, perms)) < 0) {
    fprintf(stderr, "Error in copy_file: cannot create '%s'\n", dst_filename);
    exit(1);
  }


  if (inF >= 0) {
    while ((bytes = read(inF, buf, sizeof(buf))) > 0) {
      write(outF, buf, bytes);
    }
    close(inF);
  }
  else {
    // always print this message regardless of OKAPI_VERBOSE
    fprintf(stderr, "WARNING: cannot copy contents of '%s', creating an empty file instead\n", src_filename);
  }

  close(outF);
}


#ifdef OKAPI_STANDALONE

int main(int argc, char** argv) {
  // allow most promiscuous permissions for new files/directories
  umask(0000);

  if (argc != 4) {
    fprintf(stderr, "Error, okapi takes exactly 3 arguments: <absolute_path>, <src_prefix>, <dst_prefix>\n");
    return -1;
  }
  else {
    if (!IS_ABSPATH(argv[1])) {
      fprintf(stderr, "Error, '%s' is NOT an absolute path\n", argv[1]);
      return -1;
    }

    char* src_prefix;
    if (strlen(argv[2]) == 0) { // argv[2] can be ""
      src_prefix = "";
    }
    else {
      src_prefix = realpath_strdup(argv[2]);
    }

    if (strlen(argv[3]) == 0) {
      fprintf(stderr, "Error, argv[3] cannot be empty\n");
      return -1;
    }
    char* dst_prefix = realpath_strdup(argv[3]);

    create_mirror_file(argv[1], src_prefix, dst_prefix);
  }
  return 0;
}

#endif
