/* 
  
okapi (pronounced "oh-copy") is a robust file copying utility for Linux
that gracefully handles the utter grossness of symlinks and
sub-directories.

Created by Philip Guo on 2011-08-02

okapi is currently included as a library within the CDE project.

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

#ifndef _PATHS_H
#define _PATHS_H

#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>

//#define _GNU_SOURCE // for vasprintf (now we include _GNU_SOURCE in Makefile)
#include <stdio.h>


// quick check for whether a path is absolute
#define IS_ABSPATH(p) ((p) && p[0] == '/')

// to shut up gcc warnings without going thru #include hell
extern char* basename(const char *fname);
extern char *dirname(char *path);

char* format(const char *format, ...);

char* realpath_strdup(char* filename);
char* readlink_strdup(char* filename);

char* realpath_nofollow_DEPRECATED(char* filename, char* relative_path_basedir);

int file_is_within_dir(char* filename, char* target_dir, char* relative_path_basedir);

void mkdir_recursive(char* fullpath, int pop_one);

// adapted from Goanna project

/* A structure to represent paths. */
struct namecomp {
  int len;
  char str[0];
};

struct path {
  int stacksize; // num elts in stack
  int depth;     // actual depth of path (smaller than stacksize)
  int is_abspath; // 1 if this represents an absolute path, 0 otherwise
  struct namecomp **stack;
};


char* get_path_component(struct path* p, int ind);

char* canonicalize_abspath(char* abspath);
char* canonicalize_path(char* path, char* relpath_base);

struct path* new_path_from_abspath(char* path);
struct path* new_path_from_relpath(char* relpath, char* base);

char* path2str(struct path* path, int depth);
void delete_path(struct path *path);

void create_mirror_file(char* filename_abspath, char* src_prefix, char* dst_prefix);
void create_mirror_dirs(char* original_abspath, char* src_prefix, char* dst_prefix, int pop_one);
void create_mirror_symlink_and_target(char* filename_abspath, char* src_prefix, char* dst_prefix);
void copy_file(char* src_filename, char* dst_filename, int perms);

#endif // _PATHS_H
