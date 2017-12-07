/*

CDE: Code, Data, and Environment packaging for Linux
http://www.stanford.edu/~pgbovine/cde.html
Philip Guo

CDE is currently licensed under GPL v3:

  Copyright (c) 2010-2011 Philip Guo <pg@cs.stanford.edu>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

*/


/* Linux system call calling conventions:

   According to this page:
     http://stackoverflow.com/questions/2535989/what-are-the-calling-conventions-for-unix-linux-system-calls-on-x86-64

   ... and the source code for systrace: http://www.citi.umich.edu/u/provos/systrace/

  32-bit x86:
    syscall number: %eax
    first 6 syscall parameters: %ebx, %ecx, %edx, %esi, %edi, %ebp

  64-bit x86-64:
    syscall number: %rax
    first 6 syscall parameters (for a 64-bit target process): %rdi, %rsi, %rdx, %rcx, %r8 and %r9
    first 6 syscall parameters (for a 32-bit target process): %rbx, %rcx, %rdx, %rsi, %rdi, %rbp
      (note how these are similar to the 32-bit syscall parameter registers)

*/

#include "cde.h"
#include "okapi.h"
#include <dirent.h>

// for CDE_begin_socket_bind_or_connect
#include <sys/socket.h>
#include <sys/un.h>

#include <time.h>

#include <sys/utsname.h> // for uname

// TODO: eliminate this hack if it results in a compile-time error
#include "config.h" // to get I386 / X86_64 definitions
#if defined (I386)
__asm__(".symver shmctl,shmctl@GLIBC_2.0"); // hack to eliminate glibc 2.2 dependency
#endif


// 1 if we are executing code in a CDE package,
// 0 for tracing regular execution
char CDE_exec_mode;

char CDE_verbose_mode = 0; // -v option

// only valid if !CDE_exec_mode
char* CDE_PACKAGE_DIR = NULL;
char* CDE_ROOT_DIR = NULL;

char CDE_block_net_access = 0; // -n option


// only relevant if CDE_exec_mode = 1
char CDE_exec_streaming_mode = 0; // -s option


#if defined(X86_64)
// current_personality == 1 means that a 64-bit cde-exec is actually tracking a
// 32-bit target process at the moment:
#define IS_32BIT_EMU (current_personality == 1)
#endif

// Super-simple trie implementation for doing fast string matching:
// adapted from my earlier IncPy project

typedef struct _trie {
  struct _trie* children[128]; // we support ASCII characters from 0 to 127
  int elt_is_present; // 1 if there is an element present here
} Trie;


static Trie* TrieNew(void) {
  // VERY important to blank out the contents with a calloc()
  return (Trie*)calloc(1, sizeof(Trie));
}

/* currently unused ... but could be useful in the future
static void TrieDelete(Trie* t) {
  // free all your children before freeing yourself
  unsigned char i;
  for (i = 0; i < 128; i++) {
    if (t->children[i]) {
      TrieDelete(t->children[i]);
    }
  }
  free(t);
}
*/

static void TrieInsert(Trie* t, char* ascii_string) {
  while (*ascii_string != '\0') {
    unsigned char idx = (unsigned char)*ascii_string;
    assert(idx < 128); // we don't support extended ASCII characters
    if (!t->children[idx]) {
      t->children[idx] = TrieNew();
    }
    t = t->children[idx];
    ascii_string++;
  }

  t->elt_is_present = 1;
}

static int TrieContains(Trie* t, char* ascii_string) {
  while (*ascii_string != '\0') {
    unsigned char idx = (unsigned char)*ascii_string;
    t = t->children[idx];
    if (!t) {
      return 0; // early termination, no match!
    }
    ascii_string++;
  }

  return t->elt_is_present;
}


// 1 if we should use the dynamic linker from within the package
//   (much more portable, but might be less robust since the dynamic linker
//   must be invoked explicitly, which leads to some weird-ass bugs)
// 0 if we should attempt to use the native dynamic linker from target machine
//   (not portable at all since the target machine's dynamic linker must
//   match the libc version WITHIN the package, but potentially more
//   robust if the target and source machines are identically-configured)
char CDE_use_linker_from_package = 1; // ON by default, -l option to turn OFF

// only 1 if we are running cde-exec from OUTSIDE of a cde-root/ directory
char cde_exec_from_outside_cderoot = 0;

FILE* CDE_copied_files_logfile = NULL;

static char cde_options_initialized = 0; // set to 1 after CDE_init_options() done

static void begin_setup_shmat(struct tcb* tcp);
static void* find_free_addr(int pid, int exec, unsigned long size);

static char* strcpy_from_child(struct tcb* tcp, long addr);
static char* strcpy_from_child_or_null(struct tcb* tcp, long addr);
static int ignore_path(char* filename, struct tcb* tcp);

#define SHARED_PAGE_SIZE (MAXPATHLEN * 4)

static char* redirect_filename_into_cderoot(char* filename, char* child_current_pwd, struct tcb* tcp);
static void memcpy_to_child(int pid, char* dst_child, char* src, int size);


// the true pwd of the cde executable AT THE START of execution
char cde_starting_pwd[MAXPATHLEN];

// these arrays are initialized in CDE_init_options()
// yeah, statically-sized arrays are dumb but easy to implement :)
static char* ignore_exact_paths[100];
static char* ignore_prefix_paths[100];
static char* ignore_substr_paths[100];
int ignore_exact_paths_ind = 0;
int ignore_prefix_paths_ind = 0;
int ignore_substr_paths_ind = 0;

// these override their ignore path counterparts
static char* redirect_exact_paths[100];
static char* redirect_prefix_paths[100];
static char* redirect_substr_paths[100];
int redirect_exact_paths_ind = 0;
int redirect_prefix_paths_ind = 0;
int redirect_substr_paths_ind = 0;

static char* ignore_envvars[100]; // each element should be an environment variable to ignore
int ignore_envvars_ind = 0;

struct PI process_ignores[50];
int process_ignores_ind = 0;


// the absolute path to the cde-root/ directory, since that will be
// where our fake filesystem starts. e.g., if cde_starting_pwd is
//   /home/bob/cde-package/cde-root/home/alice/cool-experiment
// then cde_pseudo_root_dir is:
//   /home/bob/cde-package/cde-root
//
// only relevant when we're executing in CDE_exec_mode
char cde_pseudo_root_dir[MAXPATHLEN];


// the path to where the root directory is mounted on the remote machine
// (only relevant for "cde-exec -s")
char* cde_remote_root_dir = NULL;
// file paths that should be accessed in cde-package/cde-root/
// rather than on the remote machine (only relevant for "cde-exec -s")
static Trie* cached_files_trie = NULL;
FILE* cached_files_fp = NULL; // save cached_files_trie on-disk as "locally-cached-files.txt"


// to shut up gcc warnings without going thru #include hell
extern ssize_t getline(char **lineptr, size_t *n, FILE *stream);

extern char* find_ELF_program_interpreter(char * file_name); // from ../readelf-mini/libreadelf-mini.a

extern void path_pop(struct path* p);


static void CDE_init_options(void);
static void CDE_create_convenience_scripts(char** argv, int optind);
static void CDE_create_toplevel_symlink_dirs(void);
static void CDE_create_path_symlink_dirs(void);
static void CDE_load_environment_vars(void);



// returns a component within real_pwd that represents the part within
// cde_pseudo_root_dir
// the return value should NOT be mutated; otherwise we might be screwed!
//
// (tcp argument is optional and used to pass into ignore_path)
static char* extract_sandboxed_pwd(char* real_pwd, struct tcb* tcp) {
  assert(CDE_exec_mode);

  // spoof getcwd by only taking the part BELOW cde-root/
  // e.g., if real_pwd is:
  //   /home/bob/cde-package/cde-root/home/alice/cool-experiment
  // then return:
  //   /home/alice/cool-experiment
  // as cwd
  int cde_pseudo_root_dir_len = strlen(cde_pseudo_root_dir);

  char real_pwd_is_within_cde_pseudo_root_dir =
    ((strlen(real_pwd) >= cde_pseudo_root_dir_len) &&
     (strncmp(real_pwd, cde_pseudo_root_dir, cde_pseudo_root_dir_len) == 0));

  // if real_pwd is within a strange directory like '/tmp' that should
  // be ignored, AND if it resides OUTSIDE of cde_pseudo_root_dir, then
  // simply return itself
  //
  // e.g., if real_pwd is '/tmp', then return itself,
  // but if real_pwd is '/tmp/cde-package/cde-root/home/pgbovine' and
  // cde_pseudo_root_dir is '/tmp/cde-package/cde-root/', then
  // treat it like any normal path (extract '/home/pgbovine')
  if (ignore_path(real_pwd, tcp) && !real_pwd_is_within_cde_pseudo_root_dir) {
    return real_pwd;
  }

  // sanity check, make sure real_pwd is within/ cde_pseudo_root_dir,
  // if we're not ignoring it
  if (!real_pwd_is_within_cde_pseudo_root_dir) {
    // if we're in this mode, then we're okay!!!  don't return an error!
    if (cde_exec_from_outside_cderoot) {
      return real_pwd;
    }
    else {
      fprintf(stderr,
              "Fatal error: '%s' is outside of cde-root/ and NOT being ignored.\n",
              real_pwd);
      exit(1);
    }
  }

  // regular action: truncate path up to and including 'cde-root/'

  char* sandboxed_pwd = (real_pwd + cde_pseudo_root_dir_len);

  // special case for '/' directory:
  if (strlen(sandboxed_pwd) == 0) {
    return (char*)"/";
  }
  else {
    return sandboxed_pwd;
  }
}


// prepend CDE_ROOT_DIR to the given path string, assumes that the string
// starts with '/' (i.e., it's an absolute path)
// (mallocs a new string)
char* prepend_cderoot(char* path) {
  assert(IS_ABSPATH(path));
  return format("%s%s", CDE_ROOT_DIR, path);
}

// WARNING: this function behaves differently depending on value of CDE_exec_mode
char* create_abspath_within_cderoot(char* path) {
  assert(IS_ABSPATH(path)); // Pre-req: path must be an absolute path!

  if (CDE_exec_mode) {
    // if we're making a cde-exec run, then simply re-route it
    // inside of cde_pseudo_root_dir

    /* SUPER WEIRD special case: Sometimes 'path' will ALREADY BE within
       cde_pseudo_root_dir, so in those cases, do NOT redirect it again.
       Instead, simply strdup the original path (and maybe issue a warning).

       This can happen if, say, the target program reads /proc/self/maps
       or /proc/<pid>/maps and extracts the final field in a line, which
       represents the filename of a file that's been mmapped into the
       process's address space.  If we're running in cde-exec mode, then
       the filename extracted from the maps 'pseudo-file' is actually an
       absolute path WITHIN cde-root/.  e.g.,:

       00754000-00755000 rw-p 00165000 08:01 85299      /home/pgbovine/cde-package/cde-root/bin/foo

       If we try to blindly redirect this path within cde-root/ again,
       we'll get something nonsensical like:

       /home/pgbovine/cde-package/cde-root/home/pgbovine/cde-package/cde-root/bin/foo

       To prevent such atrocities, we just do a simple check to see if a
       path is already within cde-root/, and if so, then don't redirect it.
      
    */
    if(strncmp(path, cde_pseudo_root_dir, strlen(cde_pseudo_root_dir)) == 0) {
      // TODO: maybe print a warning to stderr or a log file?
      //fprintf(stderr, "CDE WARNING: refusing to redirect path that's within cde-root/: '%s'", path);
      return strdup(path);
    }
    else {
      if (CDE_exec_streaming_mode) {
        // copy file into local cde-root/ 'cache' (if necessary)

        // we REALLY rely on cached_files_trie for performance to avoid
        // unnecessary filesystem accesses
        if (TrieContains(cached_files_trie, path)) {
          // cache hit!  fall-through
        }
        else {
          printf("Accessing remote file: '%s'\n", path);
          // copy from remote -> local
          create_mirror_file(path, cde_remote_root_dir, cde_pseudo_root_dir);

          // VERY IMPORTANT: add ALL paths to cached_files_trie, even
          // for nonexistent files, so that we can avoid trying to access
          // those nonexistent files on the remote machine in future
          // executions.  Remember, ANY filesystem access we can avoid
          // will lead to speed-ups.
          TrieInsert(cached_files_trie, path);

          if (cached_files_fp) {
            fprintf(cached_files_fp, "%s\n", path);
          }
        }
      }

      // normal behavior - redirect into cde-root/
      return format("%s%s", cde_pseudo_root_dir, path);
    }
  }
  else {
    // if we're making an ORIGINAL (tracing) run, then simply prepend
    // CDE_ROOT_DIR to path and canonicalize it
    char* path_within_cde_root = prepend_cderoot(path);

    // really really tricky ;)  if the child process has changed
    // directories, then we can't rely on path_within_cde_root to
    // exist.  instead, we must create an ABSOLUTE path based on
    // cde_starting_pwd, which is the directory where cde-exec was first launched!
    char* ret = canonicalize_path(path_within_cde_root, cde_starting_pwd);
    free(path_within_cde_root);

    assert(IS_ABSPATH(ret));
    return ret;
  }
}



// original_abspath must be an absolute path
// create all the corresponding 'mirror' directories within
// cde-package/cde-root/, MAKING SURE TO CREATE DIRECTORY SYMLINKS
// when necessary (sort of emulate "mkdir -p" functionality)
// if pop_one is non-zero, then pop last element before doing "mkdir -p"
static void make_mirror_dirs_in_cde_package(char* original_abspath, int pop_one) {
  create_mirror_dirs(original_abspath, (char*)"", CDE_ROOT_DIR, pop_one);
}


// does simple string comparisons on ABSOLUTE PATHS.
// (tcp argument is optional and used for tcp->p_ignores)
static int ignore_path(char* filename, struct tcb* tcp) {
  assert(cde_options_initialized);

  // sometimes you will get a BOGUS empty filename ... in that case,
  // simply ignore it (this might hide some true errors, though!!!)
  if (filename[0] == '\0') {
    return 1;
  }

  assert(IS_ABSPATH(filename));

  int i;

  // process-specific ignores take precedence over global ignores
  // remember, tcp is optional
  if (tcp && tcp->p_ignores) {
    if (strcmp(filename, tcp->p_ignores->process_name) == 0) {
      if (CDE_verbose_mode) {
        printf("IGNORED '%s' (process=%s)\n", filename, tcp->p_ignores->process_name);
      }
      return 1;
    }
    for (i = 0; i < tcp->p_ignores->process_ignore_prefix_paths_ind; i++) {
      char* p = tcp->p_ignores->process_ignore_prefix_paths[i];
      if (strncmp(filename, p, strlen(p)) == 0) {
        if (CDE_verbose_mode) {
          printf("IGNORED '%s' [%s] (process=%s)\n", filename, p, tcp->p_ignores->process_name);
        }
        return 1;
      }
    }
  }


  // redirect paths override ignore paths
  for (i = 0; i < redirect_exact_paths_ind; i++) {
    if (strcmp(filename, redirect_exact_paths[i]) == 0) {
      return 0;
    }
  }
  for (i = 0; i < redirect_prefix_paths_ind; i++) {
    char* p = redirect_prefix_paths[i];
    if (strncmp(filename, p, strlen(p)) == 0) {
      return 0;
    }
  }
  for (i = 0; i < redirect_substr_paths_ind; i++) {
    if (strstr(filename, redirect_substr_paths[i])) {
      return 0;
    }
  }


  for (i = 0; i < ignore_exact_paths_ind; i++) {
    if (strcmp(filename, ignore_exact_paths[i]) == 0) {
      return 1;
    }
  }
  for (i = 0; i < ignore_prefix_paths_ind; i++) {
    char* p = ignore_prefix_paths[i];
    if (strncmp(filename, p, strlen(p)) == 0) {
      return 1;
    }
  }
  for (i = 0; i < ignore_substr_paths_ind; i++) {
    if (strstr(filename, ignore_substr_paths[i])) {
      return 1;
    }
  }

  if (cde_exec_from_outside_cderoot) {
    // if we're running cde-exec from OUTSIDE of cde-root/, then adopt a
    // 'Union FS' like policy where if a version of the file exists
    // within cde-package/cde-root/, then use it (return 0 to NOT
    // ignore), otherwise try using the version in the real system
    // directory (return 1 to ignore)
    struct stat tmp_statbuf;
    char* redirected_filename = create_abspath_within_cderoot(filename);
    if (stat(redirected_filename, &tmp_statbuf) == 0) {
      free(redirected_filename);
      return 0;
    }
    else {
      free(redirected_filename);
      return 1;
    }
  }
  else {
    // do NOT ignore by default.  if you want to ignore everything except
    // for what's explicitly specified by 'redirect' directives, then
    // use an option like "ignore_prefix=/" (to ignore everything) and
    // then add redirect_prefix= and redirect_exact= directives accordingly
    return 0;
  }
}


// copies a file into its respective location within cde-root/,
// creating all necessary intermediate sub-directories and symlinks
//
// if filename is a symlink, then copy both it AND its target into cde-root
static void copy_file_into_cde_root(char* filename, char* child_current_pwd) {
  assert(filename);
  assert(!CDE_exec_mode);

  // resolve absolute path relative to child_current_pwd and
  // get rid of '..', '.', and other weird symbols
  char* filename_abspath = canonicalize_path(filename, child_current_pwd);

  // don't copy filename that we're ignoring (remember to use ABSOLUTE PATH)
  if (ignore_path(filename_abspath, NULL)) {
    free(filename_abspath);
    return;
  }

  if (CDE_copied_files_logfile) {
    fprintf(CDE_copied_files_logfile, "%s\n", filename_abspath);
  }

  create_mirror_file(filename_abspath, (char*)"", CDE_ROOT_DIR);

  free(filename_abspath);
}


extern int isascii(int c);
extern int isprint(int c);
extern int isspace(int c);

#define STRING_ISGRAPHIC(c) ( ((c) == '\t' || (isascii (c) && isprint (c))) )


// modify a single argument to the given system call
// to a path within cde-root/, if applicable
//
// arg_num == 1 mean modify first register arg
// arg_num == 2 mean modify second register arg
static void modify_syscall_single_arg(struct tcb* tcp, int arg_num, char* filename) {
  assert(CDE_exec_mode);
  assert(filename);

  char* redirected_filename =
    redirect_filename_into_cderoot(filename, tcp->current_dir, tcp);
  if (!redirected_filename) {
    return;
  }

  if (!tcp->childshm) {
    begin_setup_shmat(tcp);

    // no more need for filename, so don't leak it
    free(redirected_filename);

    return; // MUST punt early here!!!
  }

  // redirect all requests for absolute paths to version within cde-root/
  // if those files exist!

  strcpy(tcp->localshm, redirected_filename); // hopefully this doesn't overflow :0

  //printf("  redirect %s\n", tcp->localshm);
  //static char tmp[MAXPATHLEN];
  //EXITIF(umovestr(tcp, (long)tcp->childshm, sizeof tmp, tmp) < 0);
  //printf("     %s\n", tmp);

  struct user_regs_struct cur_regs;
  EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

  if (arg_num == 1) {
#if defined (I386)
    cur_regs.ebx = (long)tcp->childshm;
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rbx = (long)tcp->childshm;
    }
    else {
      cur_regs.rdi = (long)tcp->childshm;
    }
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif
  }
  else {
    assert(arg_num == 2);
#if defined (I386)
    cur_regs.ecx = (long)tcp->childshm;
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rcx = (long)tcp->childshm;
    }
    else {
      cur_regs.rsi = (long)tcp->childshm;
    }
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif
  }

  ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);

  free(redirected_filename);
}


// copy and paste from modify_syscall_first_arg ;)
static void modify_syscall_two_args(struct tcb* tcp) {
  assert(CDE_exec_mode);

  if (!tcp->childshm) {
    begin_setup_shmat(tcp);
    return; // MUST punt early here!!!
  }

  char* filename1 = strcpy_from_child(tcp, tcp->u_arg[0]);
  char* redirected_filename1 =
    redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
  free(filename1);

  char* filename2 = strcpy_from_child(tcp, tcp->u_arg[1]);
  char* redirected_filename2 =
    redirect_filename_into_cderoot(filename2, tcp->current_dir, tcp);
  free(filename2);

  // gotta do both, yuck
  if (redirected_filename1 && redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename1);

    int len1 = strlen(redirected_filename1);
    char* redirect_file2_begin = ((char*)tcp->localshm) + len1 + 1;
    strcpy(redirect_file2_begin, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ebx = (long)tcp->childshm;
    cur_regs.ecx = (long)(((char*)tcp->childshm) + len1 + 1);
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rbx = (long)tcp->childshm;
      cur_regs.rcx = (long)(((char*)tcp->childshm) + len1 + 1);
    }
    else {
      cur_regs.rdi = (long)tcp->childshm;
      cur_regs.rsi = (long)(((char*)tcp->childshm) + len1 + 1);
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);

    //static char tmp[MAXPATHLEN];
    //EXITIF(umovestr(tcp, (long)cur_regs.ebx, sizeof tmp, tmp) < 0);
    //printf("  ebx: %s\n", tmp);
    //EXITIF(umovestr(tcp, (long)cur_regs.ecx, sizeof tmp, tmp) < 0);
    //printf("  ecx: %s\n", tmp);
  }
  else if (redirected_filename1) {
    strcpy(tcp->localshm, redirected_filename1);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ebx = (long)tcp->childshm; // only set EBX
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rbx = (long)tcp->childshm;
    }
    else {
      cur_regs.rdi = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }
  else if (redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ecx = (long)tcp->childshm; // only set ECX
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rcx = (long)tcp->childshm;
    }
    else {
      cur_regs.rsi = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }

  if (redirected_filename1) free(redirected_filename1);
  if (redirected_filename2) free(redirected_filename2);
}

// modify the second and fourth args to redirect into cde-root/
// really nasty copy-and-paste from modify_syscall_two_args above
static void modify_syscall_second_and_fourth_args(struct tcb* tcp) {
  assert(CDE_exec_mode);

  if (!tcp->childshm) {
    begin_setup_shmat(tcp);
    return; // MUST punt early here!!!
  }

  char* filename1 = strcpy_from_child(tcp, tcp->u_arg[1]);
  char* redirected_filename1 =
    redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
  free(filename1);

  char* filename2 = strcpy_from_child(tcp, tcp->u_arg[3]);
  char* redirected_filename2 =
    redirect_filename_into_cderoot(filename2, tcp->current_dir, tcp);
  free(filename2);

  // gotta do both, yuck
  if (redirected_filename1 && redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename1);

    int len1 = strlen(redirected_filename1);
    char* redirect_file2_begin = ((char*)tcp->localshm) + len1 + 1;
    strcpy(redirect_file2_begin, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ecx = (long)tcp->childshm;
    cur_regs.esi = (long)(((char*)tcp->childshm) + len1 + 1);
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rcx = (long)tcp->childshm;
      cur_regs.rsi = (long)(((char*)tcp->childshm) + len1 + 1);
    }
    else {
      cur_regs.rsi = (long)tcp->childshm;
      cur_regs.rcx = (long)(((char*)tcp->childshm) + len1 + 1);
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }
  else if (redirected_filename1) {
    strcpy(tcp->localshm, redirected_filename1);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ecx = (long)tcp->childshm;
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rcx = (long)tcp->childshm;
    }
    else {
      cur_regs.rsi = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }
  else if (redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.esi = (long)tcp->childshm; // only set ECX
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rsi = (long)tcp->childshm;
    }
    else {
      cur_regs.rcx = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }

  if (redirected_filename1) free(redirected_filename1);
  if (redirected_filename2) free(redirected_filename2);
}

// modify the first and third args to redirect into cde-root/
// really nasty copy-and-paste from modify_syscall_two_args above
static void modify_syscall_first_and_third_args(struct tcb* tcp) {
  assert(CDE_exec_mode);

  if (!tcp->childshm) {
    begin_setup_shmat(tcp);
    return; // MUST punt early here!!!
  }

  char* filename1 = strcpy_from_child(tcp, tcp->u_arg[0]);
  char* redirected_filename1 =
    redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
  free(filename1);

  char* filename2 = strcpy_from_child(tcp, tcp->u_arg[2]);
  char* redirected_filename2 =
    redirect_filename_into_cderoot(filename2, tcp->current_dir, tcp);
  free(filename2);

  // gotta do both, yuck
  if (redirected_filename1 && redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename1);

    int len1 = strlen(redirected_filename1);
    char* redirect_file2_begin = ((char*)tcp->localshm) + len1 + 1;
    strcpy(redirect_file2_begin, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ebx = (long)tcp->childshm;
    cur_regs.edx = (long)(((char*)tcp->childshm) + len1 + 1);
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rbx = (long)tcp->childshm;
      cur_regs.rdx = (long)(((char*)tcp->childshm) + len1 + 1);
    }
    else {
      cur_regs.rdi = (long)tcp->childshm;
      cur_regs.rdx = (long)(((char*)tcp->childshm) + len1 + 1);
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }
  else if (redirected_filename1) {
    strcpy(tcp->localshm, redirected_filename1);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.ebx = (long)tcp->childshm;
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rbx = (long)tcp->childshm;
    }
    else {
      cur_regs.rdi = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }
  else if (redirected_filename2) {
    strcpy(tcp->localshm, redirected_filename2);

    struct user_regs_struct cur_regs;
    EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
    cur_regs.edx = (long)tcp->childshm; // only set ECX
#elif defined(X86_64)
    if (IS_32BIT_EMU) {
      cur_regs.rdx = (long)tcp->childshm;
    }
    else {
      cur_regs.rdx = (long)tcp->childshm;
    }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

    ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
  }

  if (redirected_filename1) free(redirected_filename1);
  if (redirected_filename2) free(redirected_filename2);
}


// create a malloc'ed filename that contains a version within cde-root/
// return NULL if the filename should NOT be redirected
// WARNING: behavior differs based on CDE_exec_mode!
//
// (tcp argument is optional and used to pass into ignore_path)
static char* redirect_filename_into_cderoot(char* filename, char* child_current_pwd, struct tcb* tcp) {
  /* sometimes this is called with a null arg ... investigate further
     before making this hack permanent, though
  if (!filename) {
    return NULL;
  }
  */
  assert(filename);
  assert(child_current_pwd);

  char* filename_abspath = NULL;
  if (CDE_exec_mode) {
    // canonicalize_path has the desirable side effect of preventing
    // 'malicious' paths from going below the pseudo-root '/' ... e.g.,
    // if filename is '/home/pgbovine/../../../../'
    // then filename_abspath is simply '/'
    //
    // we resolve relative paths w.r.t.
    // extract_sandboxed_pwd(child_current_pwd), so that programs
    // can't use relative paths like '../../../' to get out of sandbox
    //
    // this is why it's VERY IMPORTANT to canonicalize before creating a
    // path into CDE_ROOT_DIR, so that absolute paths can't 'escape'
    // the sandbox
    filename_abspath =
      canonicalize_path(filename, extract_sandboxed_pwd(child_current_pwd, tcp));
  }
  else {
    filename_abspath = canonicalize_path(filename, child_current_pwd);
  }
  assert(filename_abspath);


  // don't redirect paths that we're ignoring (remember to use ABSOLUTE PATH)
  if (ignore_path(filename_abspath, tcp)) {
    free(filename_abspath);
    return NULL;
  }


  // WARNING: behavior of create_abspath_within_cderoot
  // differs based on CDE_exec_mode!
  char* ret = create_abspath_within_cderoot(filename_abspath);

  if (CDE_verbose_mode) {
    printf("redirect '%s' => '%s'\n", filename, ret);
  }

  free(filename_abspath);
  return ret;
}


/* standard functionality for syscalls that take a filename as first argument

  cde (package creation) mode:
    - if abspath(filename) is outside pwd, then copy it into cde-root/

  cde-exec mode:
    - if abspath(filename) is outside pwd, then redirect it into cde-root/

sys_open(filename, flags, mode)
sys_creat(filename, mode)
sys_chmod(filename, ...)
sys_chown(filename, ...)
sys_chown16(filename, ...)
sys_lchown(filename, ...)
sys_lchown16(filename, ...)
sys_stat(filename, ...)
sys_stat64(filename, ...)
sys_lstat(filename, ...)
sys_lstat64(filename, ...)
sys_truncate(path, length)
sys_truncate64(path, length)
sys_access(filename, mode)
sys_utime(filename, ...)
sys_readlink(path, ...)

 */
void CDE_begin_standard_fileop(struct tcb* tcp, const char* syscall_name) {
  //char* filename = strcpy_from_child(tcp, tcp->u_arg[0]);

  /* Patch by Edward Wang
     "Attached is a patch to fix a small bug that happens when a syscall
     is called without any arguments (tcp->u_arg[0] is "0"). This
     happened to me a few times when I was trying to package a portable
     version of VLC media player."
  */
  char* filename = strcpy_from_child_or_null(tcp, tcp->u_arg[0]);
  if (filename == NULL)
    return;

  if (CDE_verbose_mode) {
    printf("[%d] BEGIN %s '%s'\n", tcp->pid, syscall_name, filename);
  }

  if (CDE_exec_mode) {
    if (filename) {
      modify_syscall_single_arg(tcp, 1, filename);
    }
  }
  else {
    // pre-emptively copy the given file into cde-root/, silencing warnings for
    // non-existent files.
    // (Note that filename can sometimes be a JUNKY STRING due to weird race
    //  conditions when strace is tracing complex multi-process applications)
    if (filename) {
      copy_file_into_cde_root(filename, tcp->current_dir);
    }
  }

  free(filename);
}


/* standard functionality for *at syscalls that take a dirfd as first
   argument, followed by a filepath
   e.g., see documentation for http://linux.die.net/man/2/openat

  example syscalls:
    openat,faccessat,fstatat64,fchownat,fchmodat,futimesat,mknodat

  if filepath is an absolute path, or if filepath is a relative path but
  dirfd is AT_FDCWD, then:

  cde (package creation) mode:
    - if abspath(filepath) is outside pwd, then copy it into cde-root/

  exec mode:
    - if abspath(filepath) is outside pwd, then redirect it into cde-root/

  issue a warning if filepath is a relative path but dirfd is NOT AT_FDCWD
*/
void CDE_begin_at_fileop(struct tcb* tcp, const char* syscall_name) {
  char* filename = strcpy_from_child(tcp, tcp->u_arg[1]);

  if (CDE_verbose_mode) {
    printf("[%d] BEGIN %s '%s' (dirfd=%u)\n", tcp->pid, syscall_name, filename, (unsigned int)tcp->u_arg[0]);
  }

  if (!IS_ABSPATH(filename) && tcp->u_arg[0] != AT_FDCWD) {
    fprintf(stderr,
            "CDE WARNING (unsupported operation): %s '%s' is a relative path and dirfd != AT_FDCWD\n",
            syscall_name, filename);
    goto done; // punt early!
  }

  if (CDE_exec_mode) {
    modify_syscall_single_arg(tcp, 2, filename);
  }
  else {
    // pre-emptively copy the given file into cde-root/, silencing warnings for
    // non-existent files.
    // (Note that filename can sometimes be a JUNKY STRING due to weird race
    //  conditions when strace is tracing complex multi-process applications)
    copy_file_into_cde_root(filename, tcp->current_dir);
  }

done:
  free(filename);
}


// input_buffer_arg_index is the index of the input filename argument
// output_buffer_arg_index is the index of the argument where the output
// buffer is being held (we clobber this in some special cases)
static void CDE_end_readlink_internal(struct tcb* tcp, int input_buffer_arg_index, int output_buffer_arg_index) {
  char* filename = strcpy_from_child(tcp, tcp->u_arg[input_buffer_arg_index]);

  if (CDE_exec_mode) {
    if (tcp->u_rval >= 0) {
      // super hack!  if the program is trying to access the special
      // /proc/self/exe file, return perceived_program_fullpath if
      // available, or else cde-exec will ERRONEOUSLY return the path
      // to the dynamic linker (e.g., ld-linux.so.2).
      //
      // programs like 'java' rely on the value of /proc/self/exe
      // being the true path to the executable, in order to dynamically
      // load libraries based on paths relative to that full path!
      char is_proc_self_exe = (strcmp(filename, "/proc/self/exe") == 0);

      // another super hack!  programs like Google Earth
      // ('googleearth-bin') access /proc/self/exe as /proc/<pid>/exe
      // where <pid> is ITS OWN PID!  be sure to handle that case properly
      // (but don't worry about handling cases where <pid> is the PID of
      // another process).
      //
      // (again, these programs use the real path of /proc/<pid>/exe as
      // a basis for dynamically loading libraries, so we must properly
      // 'fake' this value)
      char* self_pid_name = format("/proc/%d/exe", tcp->pid);
      char is_proc_self_pid_exe = (strcmp(filename, self_pid_name) == 0);
      free(self_pid_name);

      if ((is_proc_self_exe || is_proc_self_pid_exe) &&
          tcp->perceived_program_fullpath) {
        memcpy_to_child(tcp->pid, (char*)tcp->u_arg[output_buffer_arg_index],
                        tcp->perceived_program_fullpath,
                        strlen(tcp->perceived_program_fullpath) + 1);

        // VERY SUBTLE - set %eax (the syscall return value) to the length
        // of the FAKED STRING, since readlink is supposed to return the
        // length of the returned path (some programs like Python rely
        // on that length to allocated memory)
        struct user_regs_struct cur_regs;
        EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);
#if defined (I386)
        cur_regs.eax = (long)strlen(tcp->perceived_program_fullpath);
#elif defined(X86_64)
        cur_regs.rax = (long)strlen(tcp->perceived_program_fullpath);
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif
        ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
      }
      // if the program tries to read /proc/self/cwd, then treat it like
      // a CDE_end_getcwd call, returning a fake cwd:
      //
      // (note that we don't handle /proc/<pid>/cwd yet)
      else if (strcmp(filename, "/proc/self/cwd") == 0) {
        // copied from CDE_end_getcwd
        char* sandboxed_pwd = extract_sandboxed_pwd(tcp->current_dir, tcp);
        memcpy_to_child(tcp->pid, (char*)tcp->u_arg[output_buffer_arg_index],
                        sandboxed_pwd, strlen(sandboxed_pwd) + 1);

        // VERY SUBTLE - set %eax (the syscall return value) to the length
        // of the FAKED STRING, since readlink is supposed to return the
        // length of the returned path (some programs like Python rely
        // on that length to allocated memory)
        struct user_regs_struct cur_regs;
        EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);
#if defined (I386)
        cur_regs.eax = (long)strlen(sandboxed_pwd);
#elif defined(X86_64)
        cur_regs.rax = (long)strlen(sandboxed_pwd);
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif
        ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
      }
      else {
        // inspect the return value (stored in readlink_target) and if
        // it's a relative path that starts with './' and contains a '//'
        // marker, then it MIGHT actually be a "munged" version of an
        // absolute path symlink that was turned into a relative path
        // when the original file was copied (okapi-ed) into the package.
        // e.g., a symlink to an absolute path like /lib/libc.so.6 might
        // be munged into some monstrous relative path like:
        //
        //   ./../../../../..//lib/libc.so.6
        //
        // so that it can reference the version of /lib/libc.so.6 from
        // WITHIN THE PACKAGE rather than the native one on the target
        // machine.  However, when the target program does a readlink(),
        // it expects to the syscall to return '/lib/libc.so.6', so we
        // must properly "un-munge" these sorts of symlinks.
        //
        // (Note that we don't have this problem with symlinks to
        // relative paths.)

        // first get the length of the return value string ...
        struct user_regs_struct cur_regs;
        EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);
#if defined (I386)
        int ret_length = cur_regs.eax;
#elif defined(X86_64)
        int ret_length = cur_regs.rax;
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif

        char readlink_target[MAXPATHLEN];
        if (umoven(tcp, tcp->u_arg[output_buffer_arg_index], ret_length, readlink_target) == 0) {
          // remember to cap off the end ...
          readlink_target[ret_length] = '\0';

          // now readlink_target is the string that's "returned" by this
          // readlink syscall

          // is there a leading './' marker?
          if (strncmp(readlink_target, "./", 2) == 0) {
            // now check for a distinctive '//' marker, indicative of munged paths.
            // However, this simple check can still result in false positives!!!
            char* suffix = strstr(readlink_target, "//");
            if (suffix) {
              assert(suffix[0] == '/');
              suffix++; // skip one of the slashes

              assert(IS_ABSPATH(suffix));

              // as a final sanity check, see if this file actually exists
              // within cde_pseudo_root_dir, to prevent false positives
              char* actual_path = format("%s%s", cde_pseudo_root_dir, suffix);
              struct stat st;
              if (lstat(actual_path, &st) == 0) {
                // clobber the syscall's return value with 'suffix'
                memcpy_to_child(tcp->pid, (char*)tcp->u_arg[output_buffer_arg_index],
                                suffix, strlen(suffix) + 1);

                // VERY SUBTLE - set %eax (the syscall return value) to the length
                // of the FAKED STRING, since readlink is supposed to return the
                // length of the returned path (some programs like Python rely
                // on that length to allocated memory)
#if defined (I386)
                cur_regs.eax = (long)strlen(suffix);
#elif defined(X86_64)
                cur_regs.rax = (long)strlen(suffix);
#else
                #error "Unknown architecture (not I386 or X86_64)"
#endif
                ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
              }
              free(actual_path);
            }
          }
        }

      }
    }
  }

  free(filename);
}

void CDE_end_readlink(struct tcb* tcp) {
  // output buffer is second argument (index 1)
  CDE_end_readlink_internal(tcp, 0, 1);
}

void CDE_end_readlinkat(struct tcb* tcp) {
  // output buffer is third argument (index 2)
  CDE_end_readlink_internal(tcp, 1, 2);
}


void CDE_begin_execve(struct tcb* tcp) {
  // null all these out up-top, then deallocate them in 'done'
  char* exe_filename = NULL;
  char* redirected_path = NULL;
  char* exe_filename_abspath = NULL;
  char* script_command = NULL;
  char* ld_linux_filename = NULL;
  char* ld_linux_fullpath = NULL;

  exe_filename = strcpy_from_child(tcp, tcp->u_arg[0]);

  // only attempt to do the ld-linux.so.2 trick if exe_filename
  // is a valid executable file ... otherwise don't do
  // anything and simply let the execve fail just like it's supposed to
  struct stat filename_stat;

  // NULL out p_ignores since you might have inherited it from your parent after
  // forking, but when you exec, you're probably now executing a different program
  tcp->p_ignores = NULL;

  if (CDE_verbose_mode) {
    printf("[%d] CDE_begin_execve '%s'\n", tcp->pid, exe_filename);
  }

  if (CDE_exec_mode) {
    // if we're purposely ignoring a path to an executable (e.g.,
    // ignoring "/bin/bash" to prevent crashes on certain Ubuntu
    // machines), then DO NOT use the ld-linux trick and simply
    // execve the file normally
    //
    // (note that this check doesn't pick up the case when a textual script
    //  is being executed (e.g., with "#!/bin/bash" as its shebang line),
    //  since exe_filename is the script's name and NOT "/bin/bash".
    //  We will need to handle this case LATER in the function.)
    char* opened_filename_abspath =
      canonicalize_path(exe_filename, extract_sandboxed_pwd(tcp->current_dir, tcp));

    if (ignore_path(opened_filename_abspath, tcp)) {
      free(opened_filename_abspath);
      goto done;
    }

    // check for presence in process_ignores, and if found, set
    // tcp->p_ignores and punt
    int i;
    for (i = 0; i < process_ignores_ind; i++) {
      if (strcmp(opened_filename_abspath, process_ignores[i].process_name) == 0) {
        //printf("IGNORED '%s'\n", opened_filename_abspath);
        tcp->p_ignores = &process_ignores[i];
        free(opened_filename_abspath);
        goto done; // TOTALLY PUNT!!!
      }
    }

    free(opened_filename_abspath);

    redirected_path = redirect_filename_into_cderoot(exe_filename, tcp->current_dir, tcp);
  }

  char* path_to_executable = NULL;

  if (redirected_path) {
    // TODO: we don't check whether it's a real executable file :/
    if (stat(redirected_path, &filename_stat) != 0) {
      goto done;
    }
    path_to_executable = redirected_path;
  }
  else {
    // just check the file itself (REMEMBER TO GET ITS ABSOLUTE PATH!)
    exe_filename_abspath = canonicalize_path(exe_filename, tcp->current_dir);

    // TODO: we don't check whether it's a real executable file :/
    if (stat(exe_filename_abspath, &filename_stat) != 0) {
      goto done;
    }
    path_to_executable = exe_filename_abspath;
  }
  assert(path_to_executable);

  // WARNING: ld-linux.so.2 only works on dynamically-linked binary
  // executable files; it will fail if you invoke it on:
  //   - a textual script file
  //   - a statically-linked binary
  //
  // for a textual script file, we must invoke ld-linux.so.2 on the
  // target of the shebang #! (which can itself take arguments)
  //
  // e.g., #! /bin/sh
  // e.g., #! /usr/bin/env python
  char is_textual_script = 0;
  char is_elf_binary = 0;

  FILE* f = fopen(path_to_executable, "rb"); // open in binary mode
  assert(f);
  char header[5];
  memset(header, 0, sizeof(header));
  fgets(header, 5, f); // 5 means 4 bytes + 1 null terminating byte
  if (strcmp(header, "\177ELF") == 0) {
    is_elf_binary = 1;
  }
  fclose(f);

  if (is_elf_binary) {
    // look for whether it's a statically-linked binary ...
    // if so, then there is NO need to call ld-linux.so.2 on it;
    // we can just execute it directly (in fact, ld-linux.so.2
    // will fail on static binaries!)

    // mallocs a new string if successful
    // (this string is most likely "/lib/ld-linux.so.2")
    ld_linux_filename = find_ELF_program_interpreter(path_to_executable);
    if (!ld_linux_filename) {
      // if the program interpreter isn't found, then it's a static
      // binary, so let the execve call proceed normally
      if (CDE_exec_mode) {
        // redirect the executable's path to within $CDE_ROOT_DIR:
        modify_syscall_single_arg(tcp, 1, exe_filename);
      }
      else {
        copy_file_into_cde_root(exe_filename, tcp->current_dir);
      }

      // remember to EXIT EARLY!
      goto done;
    }
    assert(IS_ABSPATH(ld_linux_filename));
  }
  else {
    // find out whether it's a script file (starting with #! line)
    FILE* f = fopen(path_to_executable, "rb"); // open in binary mode

    size_t len = 0;
    ssize_t read;
    char* tmp = NULL; // getline() mallocs for us
    read = getline(&tmp, &len, f);
    if (read > 2) {
      assert(tmp[read-1] == '\n'); // strip of trailing newline
      tmp[read-1] = '\0'; // strip of trailing newline
      if (tmp[0] == '#' && tmp[1] == '!') {
        is_textual_script = 1;
        script_command = strdup(&tmp[2]);
      }
    }
    free(tmp);
    /* Patch from Yang Chen

       "I am packaging our tool using it. I found there is a possible
       bug in cde.c where opened files were not closed. In a long run,
       it could cause fopen fail. I noticed it because our toolchain has
       a lot of invocations on shell scripts and hence hit this
       problem.""

    */
    fclose(f);


    if (!script_command) {
      fprintf(stderr, "Fatal error: '%s' seems to be a script without a #! line.\n(cde can only execute scripts that start with a proper #! line)\n",
              path_to_executable);
      exit(1);
    }

    // now find the program interpreter for the script_command
    // executable, be sure to grab the FIRST TOKEN since that's
    // the actual executable name ...
    // TODO: this will fail if the executable's path has a space in it
    //
    // mallocs a new string if successful
    // (this string is most likely "/lib/ld-linux.so.2")

    // libc is so dumb ... strtok() alters its argument in an un-kosher way
    tmp = strdup(script_command);
    char* p = strtok(tmp, " ");

    // to have find_ELF_program_interpreter succeed, we might need to
    // redirect the path inside CDE_ROOT_DIR:
    char* script_command_filename = NULL;
    if (CDE_exec_mode) {
      // this path should look like the name in the #! line, just
      // canonicalized to be an absolute path
      char* script_command_abspath =
        canonicalize_path(p, extract_sandboxed_pwd(tcp->current_dir, tcp));

      if (ignore_path(script_command_abspath, tcp)) {
        free(script_command_abspath);
        free(tmp);
        goto done; // PUNT!
      }

      // check for presence in process_ignores, and if found, set
      // tcp->p_ignores and punt
      int i;
      for (i = 0; i < process_ignores_ind; i++) {
        if (strcmp(script_command_abspath, process_ignores[i].process_name) == 0) {
          //printf("IGNORED (script) '%s'\n", script_command_abspath);
          tcp->p_ignores = &process_ignores[i];
          free(script_command_abspath);
          free(tmp);
          goto done; // TOTALLY PUNT!!!
        }
      }

      free(script_command_abspath);

      script_command_filename = redirect_filename_into_cderoot(p, tcp->current_dir, tcp);
    }

    if (!script_command_filename) {
      script_command_filename = strdup(p);
    }

    ld_linux_filename = find_ELF_program_interpreter(script_command_filename);

    free(script_command_filename);
    free(tmp);

    if (!ld_linux_filename) {
      // if the program interpreter isn't found, then it's a static
      // binary, so let the execve call proceed unmodified

      // TODO: is this the right thing to do here?  I think we might
      // need to do something better here (think harder about this case!)
      if (CDE_exec_mode) {
        // redirect the executable's path to within cde-root/:
        modify_syscall_single_arg(tcp, 1, exe_filename);
      }

      goto done;
    }
    assert(IS_ABSPATH(ld_linux_filename));
  }

  assert(!(is_elf_binary && is_textual_script));

  if (CDE_exec_mode) {
    // set up shared memory segment if we haven't done so yet
    if (!tcp->childshm) {
      begin_setup_shmat(tcp);

      goto done; // MUST punt early here!!!
    }

    ld_linux_fullpath = create_abspath_within_cderoot(ld_linux_filename);

    /* we're gonna do some craziness here to redirect the OS to call
       cde-root/lib/ld-linux.so.2 rather than the real program, since
       ld-linux.so.2 is closely-tied with the version of libc in
       cde-root/. */
    if (is_textual_script) {
      /*  we're running a script with a shebang (#!), so
          let's set up the shared memory segment (tcp->localshm) like so:

    if (CDE_use_linker_from_package) {

    base -->       tcp->localshm : "cde-root/lib/ld-linux.so.2" (ld_linux_fullpath)
          script_command token 0 : "/usr/bin/env"
          script_command token 1 : "python"
              ... (for as many tokens as available) ...
    new_argv -->   argv pointers : point to tcp->childshm ("cde-root/lib/ld-linux.so.2")
                   argv pointers : point to script_command token 0
                   argv pointers : point to script_command token 1
              ... (for as many tokens as available) ...
                   argv pointers : point to tcp->u_arg[0] (original program name)
                   argv pointers : point to child program's argv[1]
                   argv pointers : point to child program's argv[2]
                   argv pointers : point to child program's argv[3]
                   argv pointers : [...]
                   argv pointers : NULL

    }
    else {

    base --> script_command token 0 REDIRECTED into cde-root:
                   e.g., "/home/pgbovine/cde-package/cde-root/usr/bin/env"
             script_command token 1 : "python"
              ... (for as many tokens as available) ...
    new_argv -->   argv pointers : point to script_command token 0
                   argv pointers : point to script_command token 1
              ... (for as many tokens as available) ...
                   argv pointers : point to tcp->u_arg[0] (original program name)
                   argv pointers : point to child program's argv[1]
                   argv pointers : point to child program's argv[2]
                   argv pointers : point to child program's argv[3]
                   argv pointers : [...]
                   argv pointers : NULL

    }

        Note that we only need to do this if we're in CDE_exec_mode */

      //printf("script_command='%s', path_to_executable='%s'\n", script_command, path_to_executable);

      char* base = (char*)tcp->localshm;
      int ld_linux_offset = 0;

      if (CDE_use_linker_from_package) {
        strcpy(base, ld_linux_fullpath);
        ld_linux_offset = strlen(ld_linux_fullpath) + 1;
      }

      char* cur_loc = (char*)(base + ld_linux_offset);
      char* script_command_token_starts[200]; // stores starting locations of each token

      int script_command_num_tokens = 0;

      // set this ONCE on the first token
      tcp->perceived_program_fullpath = NULL;

      // tokenize script_command into tokens, and insert them into argv
      // TODO: this will fail if the shebang line contains file paths
      // with spaces, quotes, or other weird characters!
      char* p;
      for (p = strtok(script_command, " "); p; p = strtok(NULL, " ")) {
        //printf("  token = %s\n", p);

        // set to the first token!
        if (!tcp->perceived_program_fullpath) {
          tcp->perceived_program_fullpath = strdup(p);

          // kludgy special-case handling for !CDE_use_linker_from_package mode
          //
          // set the first script_command token to a string that's
          // redirected INSIDE of cde-root ...
          if (!CDE_use_linker_from_package) {
            char* program_full_path_in_cderoot =
              redirect_filename_into_cderoot(tcp->perceived_program_fullpath, tcp->current_dir, tcp);

            strcpy(cur_loc, program_full_path_in_cderoot);
            script_command_token_starts[script_command_num_tokens] = cur_loc;
            cur_loc += (strlen(program_full_path_in_cderoot) + 1);
            script_command_num_tokens++;

            free(program_full_path_in_cderoot);

            continue;
          }
        }

        strcpy(cur_loc, p);
        script_command_token_starts[script_command_num_tokens] = cur_loc;

        cur_loc += (strlen(p) + 1);
        script_command_num_tokens++;
      }

      // We need to use raw numeric arithmetic to get the proper offsets, since
      // we need to properly handle tracing of 32-bit target programs using a
      // 64-bit cde-exec.  personality_wordsize[current_personality] gives the
      // word size for the target process (e.g., 4 bytes for a 32-bit and 8 bytes
      // for a 64-bit target process).
      unsigned long new_argv_raw = (unsigned long)(cur_loc);

      // really subtle, these addresses should be in the CHILD's address space, not the parent's

      // points to ld_linux_fullpath
      char** new_argv_0 = (char**)new_argv_raw;
      *new_argv_0 = (char*)tcp->childshm;

      if (CDE_verbose_mode) {
        char* tmp = strcpy_from_child(tcp, (long)*new_argv_0);
        printf("   new_argv[0]='%s'\n", tmp);
        if (tmp) free(tmp);
      }

      // points to all the tokens of script_command
      int i;
      for (i = 0; i < script_command_num_tokens; i++) {
        // ugly subtle indexing differences between modes :/
        if (CDE_use_linker_from_package) {
          char** new_argv_i_plus_1 = (char**)(new_argv_raw + ((i+1) * personality_wordsize[current_personality]));
          *new_argv_i_plus_1 = (char*)tcp->childshm + (script_command_token_starts[i] - base);

          if (CDE_verbose_mode) {
            char* tmp = strcpy_from_child(tcp, (long)*new_argv_i_plus_1);
            printf("   new_argv[%d]='%s'\n", i+1, tmp);
            if (tmp) free(tmp);
          }
        }
        else {
          char** new_argv_i = (char**)(new_argv_raw + (i * personality_wordsize[current_personality]));
          *new_argv_i = (char*)tcp->childshm + (script_command_token_starts[i] - base);

          if (CDE_verbose_mode) {
            char* tmp = strcpy_from_child(tcp, (long)*new_argv_i);
            printf("   new_argv[%d]='%s'\n", i, tmp);
            if (tmp) free(tmp);
          }
        }
      }

      // ugly subtle indexing differences between modes :/
      int first_nontoken_index;
      if (CDE_use_linker_from_package) {
        first_nontoken_index = script_command_num_tokens + 1;
      }
      else {
        first_nontoken_index = script_command_num_tokens;
      }

      // now populate the original program name from tcp->u_arg[0]
      char** new_argv_f = (char**)(new_argv_raw + (first_nontoken_index * personality_wordsize[current_personality]));
      *new_argv_f = (char*)tcp->u_arg[0];

      if (CDE_verbose_mode) {
        char* tmp = strcpy_from_child(tcp, (long)*new_argv_f);
        printf("   new_argv[%d]='%s'\n", first_nontoken_index, tmp);
        if (tmp) free(tmp);
      }


      // now populate argv[first_nontoken_index:] directly from child's original space
      // (original arguments)
      unsigned long child_argv_raw = (unsigned long)tcp->u_arg[1]; // in child's address space

      char* cur_arg = NULL;
      i = 1; // start at argv[1]
      while (1) {
        // read a word from child_argv_raw ...
        EXITIF(umoven(tcp,
                      (long)(child_argv_raw + (i * personality_wordsize[current_personality])),
                      personality_wordsize[current_personality],
                      (void*)&cur_arg) < 0);

        // Now set new_argv_raw[i+first_nontoken_index] = cur_arg, except the tricky part is that
        // new_argv_raw might actually be for a 32-bit target process, so if
        // we're on a 64-bit machine, we can't just use char* pointer arithmetic.
        // We must use raw numeric arithmetic to get the proper offsets.
        char** new_argv_i_plus_f = (char**)(new_argv_raw + ((i+first_nontoken_index) * personality_wordsize[current_personality]));
        *new_argv_i_plus_f = cur_arg;

        // null-terminated exit condition
        if (cur_arg == NULL) {
          break;
        }

        if (CDE_verbose_mode) {
          char* tmp = strcpy_from_child(tcp, (long)cur_arg);
          printf("   new_argv[%d]='%s'\n", i+first_nontoken_index, tmp);
          if (tmp) free(tmp);
        }

        i++;
      }

      // now set ebx to the new program name and ecx to the new argv array
      // to alter the arguments of the execv system call :0
      struct user_regs_struct cur_regs;
      EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
      cur_regs.ebx = (long)tcp->childshm;            // location of base
      cur_regs.ecx = ((long)tcp->childshm) + ((char*)new_argv_raw - base); // location of new_argv
#elif defined(X86_64)
      if (IS_32BIT_EMU) {
        cur_regs.rbx = (long)tcp->childshm;
        cur_regs.rcx = ((long)tcp->childshm) + ((char*)new_argv_raw - base);
      }
      else {
        cur_regs.rdi = (long)tcp->childshm;
        cur_regs.rsi = ((long)tcp->childshm) + ((char*)new_argv_raw - base);
      }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

      ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
    }
    else {
      /* we're running a dynamically-linked binary executable, go
         let's set up the shared memory segment (tcp->localshm) like so:

    base -->       tcp->localshm : "cde-root/lib/ld-linux.so.2" (ld_linux_fullpath)
    real_program_path_base -->   : path to target program's binary
    new_argv -->   argv pointers : point to tcp->childshm ("cde-root/lib/ld-linux.so.2")
                   argv pointers : point to tcp->childshm + strlen(ld_linux_fullpath),
                                   which is real_program_path_base in the CHILD's address space
                   argv pointers : point to child program's argv[1]
                   argv pointers : point to child program's argv[2]
                   argv pointers : point to child program's argv[3]
                   argv pointers : [...]
                   argv pointers : NULL

        Note that we only need to do this if we're in CDE_exec_mode
        and CDE_use_linker_from_package is on */

      char* base = (char*)tcp->localshm;
      strcpy(base, ld_linux_fullpath);
      int offset1 = strlen(ld_linux_fullpath) + 1;

      tcp->perceived_program_fullpath = strcpy_from_child(tcp, tcp->u_arg[0]);

      /* NOTE: we now only do this hack for 'java', since that seems to
         be the only known program that requires it (to the best of my
         knowledge).  i don't want to implement this hack for all
         programs since there are programs like 'ccache' and 'ccrypt'
         that NEED to use the original names for the program files
         (which are themselves symlinks) rather than the names resulting
         from following all symlinks.

         ok, this is super super super gross, but what we need to do is
         to set tcp->perceived_program_fullpath to the full path to the
         actual file of the target program's binary, making sure to first
         follow ALL symlinks.  otherwise programs like 'java' will fail
         since they rely on the absolute path.

         e.g., try invoking 'java' explicitly with the dynamic linker,
         and it will fail since /usr/bin/java is a symlink and not the
         path to the true binary.  'java' actually inspects the path to
         the binary in order to dynamically generate paths to libraries
         that it needs to load at start-up time ... gross!  e.g.,:

         $ /lib/ld-linux.so.2 /usr/bin/java
         /usr/bin/java: error while loading shared libraries: libjli.so: cannot open shared object file: No such file or directory

         This fails because it cannot find libjli.so on a search path
         based on /usr/bin/java.

         Since cde-exec starts up a target program by explicitly
         invoking the dynamic linker, it will face the same failure ...
         unless we pass in the absolute path to the REAL binary (not a
         symlink) to the dynamic linker.  we will do so by:

         1.) Getting the original path (tcp->u_arg[0])
         2.) Creating a version inside of cde-root/
         3.) Calling realpath() on that path in order to follow
             and resolve all symlinks
         4.) Calling extract_sandboxed_pwd() in order to get
             the version of that path back *outside* of cde-root/

       */
      if (strcmp(basename(tcp->perceived_program_fullpath), "java") == 0) {

        // create a path WITHIN cde-root, so that we can call realpath on it.
        // (otherwise this path might not exist natively on the target machine!)
        char* program_full_path_in_cderoot =
          redirect_filename_into_cderoot(tcp->perceived_program_fullpath, tcp->current_dir, tcp);

        if (program_full_path_in_cderoot) {
          // realpath follows ALL symbolic links and returns the path to the TRUE binary file :)
          char* program_realpath_in_cde_root = realpath_strdup(program_full_path_in_cderoot);
          if (program_realpath_in_cde_root) {
            // extract_sandboxed_pwd (perhaps badly named for this scenario)
            // extracts the part of program_realpath_in_cde_root that comes AFTER cde-root/
            // (note that extract_sandboxed_pwd does NOT malloc a new string)
            char* tmp_old = tcp->perceived_program_fullpath;
            tcp->perceived_program_fullpath = strdup(extract_sandboxed_pwd(program_realpath_in_cde_root, tcp));
            free(tmp_old);

            free(program_realpath_in_cde_root);
          }
          free(program_full_path_in_cderoot);
        }
      }


      char* real_program_path_base = (char*)(base + offset1);
      strcpy(real_program_path_base, tcp->perceived_program_fullpath);

      int offset2 = strlen(tcp->perceived_program_fullpath) + 1;


      // We need to use raw numeric arithmetic to get the proper offsets, since
      // we need to properly handle tracing of 32-bit target programs using a
      // 64-bit cde-exec.  personality_wordsize[current_personality] gives the
      // word size for the target process (e.g., 4 bytes for a 32-bit and 8 bytes
      // for a 64-bit target process).
      unsigned long new_argv_raw = (unsigned long)(base + offset1 + offset2);

      // really subtle, these addresses should be in the CHILD's address space, not the parent's:

      // points to ld_linux_fullpath
      char** new_argv_0 = (char**)new_argv_raw;
      *new_argv_0 = (char*)tcp->childshm;

      if (CDE_verbose_mode) {
        char* tmp = strcpy_from_child(tcp, (long)*new_argv_0);
        printf("   new_argv[0]='%s'\n", tmp);
        if (tmp) free(tmp);
      }

      char** new_argv_1 = (char**)(new_argv_raw + personality_wordsize[current_personality]);
      // points to the full path to the target program (real_program_path_base)
      *new_argv_1 = (char*)tcp->childshm + offset1;

      if (CDE_verbose_mode) {
        char* tmp = strcpy_from_child(tcp, (long)*new_argv_1);
        printf("   new_argv[1]='%s'\n", tmp);
        if (tmp) free(tmp);
      }

      // now populate argv[1:] directly from child's original space (the original arguments)
      unsigned long child_argv_raw = (unsigned long)tcp->u_arg[1]; // in child's address space
      char* cur_arg = NULL;
      int i = 1; // start at argv[1], since we're ignoring argv[0]

      while (1) {
        // read a word from child_argv_raw ...
        EXITIF(umoven(tcp,
                      (long)(child_argv_raw + (i * personality_wordsize[current_personality])),
                      personality_wordsize[current_personality],
                      (void*)&cur_arg) < 0);

        // Now set new_argv_raw[i+1] = cur_arg, except the tricky part is that
        // new_argv_raw might actually be for a 32-bit target process, so if
        // we're on a 64-bit machine, we can't just use char* pointer arithmetic.
        // We must use raw numeric arithmetic to get the proper offsets.
        char** new_argv_i_plus_1 = (char**)(new_argv_raw + ((i+1) * personality_wordsize[current_personality]));
        *new_argv_i_plus_1 = cur_arg;

        // null-terminated exit condition
        if (cur_arg == NULL) {
          break;
        }

        if (CDE_verbose_mode) {
          char* tmp = strcpy_from_child(tcp, (long)cur_arg);
          printf("   new_argv[%d]='%s'\n", i+1, tmp);
          if (tmp) free(tmp);
        }

        i++;
      }


      if (CDE_use_linker_from_package) {
        // now set ebx to the new program name and ecx to the new argv array
        // to alter the arguments of the execv system call :0
        struct user_regs_struct cur_regs;
        EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
        cur_regs.ebx = (long)tcp->childshm;            // location of base
        cur_regs.ecx = ((long)tcp->childshm) + offset1 + offset2; // location of new_argv
#elif defined(X86_64)
        if (IS_32BIT_EMU) {
          cur_regs.rbx = (long)tcp->childshm;
          cur_regs.rcx = ((long)tcp->childshm) + offset1 + offset2;
        }
        else {
          cur_regs.rdi = (long)tcp->childshm;
          cur_regs.rsi = ((long)tcp->childshm) + offset1 + offset2;
        }
#else
    #error "Unknown architecture (not I386 or X86_64)"
#endif

        ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
      }
      else {
        // simply redirect the executable's path to within cde-root/:
        modify_syscall_single_arg(tcp, 1, exe_filename);
      }
    }

    // if tcp->perceived_program_fullpath has been set, then it might be
    // a RELATIVE PATH (e.g., ./googleearth-bin), so we need to make it
    // into an ABSOLUTE PATH within cde-root/, but to only grab the
    // component that comes after cde-root/, since that's what the
    // program PERCEIVES its full path to be
    if (tcp->perceived_program_fullpath) {
      char* redirected_path =
        redirect_filename_into_cderoot(tcp->perceived_program_fullpath,
                                       tcp->current_dir, tcp);
      // redirected_path could be NULL (e.g., if it's in cde.ignore),
      // in which case just do nothing
      if (redirected_path) {
        char* old_perceived_program_fullpath = tcp->perceived_program_fullpath;

        // extract_sandboxed_pwd (perhaps badly named for this scenario)
        // extracts the part of redirected_path that comes AFTER cde-root/
        // (note that extract_sandboxed_pwd does NOT malloc a new string)
        tcp->perceived_program_fullpath =
          strdup(extract_sandboxed_pwd(redirected_path, tcp));

        free(old_perceived_program_fullpath);
      }
    }
  }
  else {
    copy_file_into_cde_root(exe_filename, tcp->current_dir);

    if (ld_linux_filename) {
      // copy ld-linux.so.2 (or whatever the program interpreter is) into cde-root
      copy_file_into_cde_root(ld_linux_filename, tcp->current_dir);
    }

    // very subtle!  if we're executing a textual script with a #!, we
    // need to grab the name of the executable from the #! string into
    // cde-root, since strace doesn't normally pick it up as a dependency
    if (is_textual_script) {
      //printf("script_command='%s', path_to_executable='%s'\n", script_command, path_to_executable);
      char* p;
      for (p = strtok(script_command, " "); p; p = strtok(NULL, " ")) {
        struct stat p_stat;
        if (stat(p, &p_stat) == 0) {
          copy_file_into_cde_root(p, tcp->current_dir);
        }
        break;
      }
    }

  }

done:
  // make sure ALL of these vars are initially set to NULL when declared:
  if (exe_filename) {
    free(exe_filename);
  }

  if (redirected_path) {
    free(redirected_path);
  }

  if (exe_filename_abspath) {
    free(exe_filename_abspath);
  }

  if (script_command) {
    free(script_command);
  }

  if (ld_linux_filename) {
    free(ld_linux_filename);
  }

  if (ld_linux_fullpath) {
    free(ld_linux_fullpath);
  }
}


void CDE_end_execve(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] CDE_end_execve\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    // WOW, what a gross hack!  execve detaches all shared memory
    // segments, so childshm is no longer valid.  we must clear it so
    // that begin_setup_shmat() will be called again
    tcp->childshm = NULL;
  }
}


void CDE_begin_file_unlink(struct tcb* tcp) {
  char* filename = strcpy_from_child(tcp, tcp->u_arg[0]);

  if (CDE_verbose_mode) {
    printf("[%d] BEGIN unlink '%s'\n", tcp->pid, filename);
  }

  if (CDE_exec_mode) {
    modify_syscall_single_arg(tcp, 1, filename);
  }
  else {
    char* redirected_path = redirect_filename_into_cderoot(filename, tcp->current_dir, tcp);
    if (redirected_path) {
      unlink(redirected_path);
      free(redirected_path);
    }
  }
}

// copy-and-paste from CDE_begin_file_unlink,
// except adjusting for unlinkat signature:
//   int unlinkat(int dirfd, const char *pathname, int flags);
void CDE_begin_file_unlinkat(struct tcb* tcp) {
  char* filename = strcpy_from_child(tcp, tcp->u_arg[1]);

  if (CDE_verbose_mode) {
    printf("[%d] BEGIN unlinkat '%s'\n", tcp->pid, filename);
  }

  if (!IS_ABSPATH(filename) && tcp->u_arg[0] != AT_FDCWD) {
    fprintf(stderr, "CDE WARNING: unlinkat '%s' is a relative path and dirfd != AT_FDCWD\n", filename);
    return; // punt early!
  }

  if (CDE_exec_mode) {
    modify_syscall_single_arg(tcp, 2, filename);
  }
  else {
    char* redirected_path = redirect_filename_into_cderoot(filename, tcp->current_dir, tcp);
    if (redirected_path) {
      unlink(redirected_path);
      free(redirected_path);
    }
  }
}


void CDE_begin_file_link(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN link\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    modify_syscall_two_args(tcp);
  }
  else {
    // just try to do the link operation within the CDE package
    // TODO: is this too early since the original link hasn't been done yet?
    // (I don't think so ...)

    char* filename1 = strcpy_from_child(tcp, tcp->u_arg[0]);
    char* redirected_filename1 =
      redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
    // first copy the origin file into cde-root/ before trying to link it
    copy_file_into_cde_root(filename1, tcp->current_dir);

    char* filename2 = strcpy_from_child(tcp, tcp->u_arg[1]);
    char* redirected_filename2 =
      redirect_filename_into_cderoot(filename2, tcp->current_dir, tcp);

    link(redirected_filename1, redirected_filename2);

    free(filename1);
    free(filename2);
    free(redirected_filename1);
    free(redirected_filename2);
  }
}


// copy-and-paste from file_link functions above,
// except adjusting for linkat signature:
//   linkat(int olddirfd, char* oldpath, int newdirfd, char* newpath, int flags);
void CDE_begin_file_linkat(struct tcb* tcp) {
  char* oldpath = strcpy_from_child(tcp, tcp->u_arg[1]);
  char* newpath = strcpy_from_child(tcp, tcp->u_arg[3]);

  if (CDE_verbose_mode) {
    printf("[%d] BEGIN linkat(%s, %s)\n", tcp->pid, oldpath, newpath);
  }

  if (!IS_ABSPATH(oldpath) && tcp->u_arg[0] != AT_FDCWD) {
    fprintf(stderr,
            "CDE WARNING: linkat '%s' is a relative path and dirfd != AT_FDCWD\n",
            oldpath);
    goto done; // punt early!
  }
  if (!IS_ABSPATH(newpath) && tcp->u_arg[2] != AT_FDCWD) {
    fprintf(stderr,
            "CDE WARNING: linkat '%s' is a relative path and dirfd != AT_FDCWD\n",
            newpath);
    goto done; // punt early!
  }


  if (CDE_exec_mode) {
    modify_syscall_second_and_fourth_args(tcp);
  }
  else {
    // just try to do the link operation within the CDE package
    // TODO: is this too early since the original link hasn't been done yet?
    // (I don't think so ...)
    //
    char* redirected_oldpath = redirect_filename_into_cderoot(oldpath, tcp->current_dir, tcp);
    // first copy the origin file into cde-root/ before trying to link it
    copy_file_into_cde_root(oldpath, tcp->current_dir);

    char* redirected_newpath = redirect_filename_into_cderoot(newpath, tcp->current_dir, tcp);

    link(redirected_oldpath, redirected_newpath);

    free(redirected_oldpath);
    free(redirected_newpath);
  }

done:
  free(oldpath);
  free(newpath);
}


void CDE_begin_file_symlink(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN symlink\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    modify_syscall_two_args(tcp);
  }
  else {
    // TODO: what about properly munging symlinks to absolute paths inside of
    // the CDE package?  e.g., if you symlink to '/lib/libc.so.6', perhaps that
    // path should be munged to '../../lib/libc.so.6' within the CDE package???
    char* oldname = strcpy_from_child(tcp, tcp->u_arg[0]);
    char* newname = strcpy_from_child(tcp, tcp->u_arg[1]);
    char* newname_redirected = redirect_filename_into_cderoot(newname, tcp->current_dir, tcp);

    symlink(oldname, newname_redirected);

    free(oldname);
    free(newname);
    free(newname_redirected);
  }
}


// copy-and-paste from above,
// except adjusting for symlinkat signature:
//   symlinkat(char* oldpath, int newdirfd, char* newpath);
void CDE_begin_file_symlinkat(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN symlinkat\n", tcp->pid);
  }

  char* newpath = strcpy_from_child(tcp, tcp->u_arg[2]);

  if (!IS_ABSPATH(newpath) && tcp->u_arg[1] != AT_FDCWD) {
    fprintf(stderr, "CDE WARNING: symlinkat '%s' is a relative path and dirfd != AT_FDCWD\n", newpath);
    free(newpath);
    return; // punt early!
  }

  if (CDE_exec_mode) {
    modify_syscall_first_and_third_args(tcp);
  }
  else {
    char* oldname = strcpy_from_child(tcp, tcp->u_arg[0]);
    char* newpath_redirected = redirect_filename_into_cderoot(newpath, tcp->current_dir, tcp);
    symlink(oldname, newpath_redirected);

    free(oldname);
    free(newpath_redirected);
  }

  free(newpath);
}


void CDE_begin_file_rename(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN rename\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    modify_syscall_two_args(tcp);
  }
}

void CDE_end_file_rename(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] END rename\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    // empty
  }
  else {
    if (tcp->u_rval == 0) {
      char* filename1 = strcpy_from_child(tcp, tcp->u_arg[0]);
      char* redirected_filename1 =
        redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
      free(filename1);
      // remove original file from cde-root/
      if (redirected_filename1) {
        unlink(redirected_filename1);
        free(redirected_filename1);
      }

      // copy the destination file into cde-root/
      char* dst_filename = strcpy_from_child(tcp, tcp->u_arg[1]);
      copy_file_into_cde_root(dst_filename, tcp->current_dir);
      free(dst_filename);
    }
  }
}


// copy-and-paste from file_rename functions above,
// except adjusting for linkat signature:
//   renameat(int olddirfd, char* oldpath, int newdirfd, char* newpath);
void CDE_begin_file_renameat(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN renameat\n", tcp->pid);
  }

  char* oldpath = strcpy_from_child(tcp, tcp->u_arg[1]);
  char* newpath = strcpy_from_child(tcp, tcp->u_arg[3]);

  if (!IS_ABSPATH(oldpath) && tcp->u_arg[0] != AT_FDCWD) {
    fprintf(stderr,
            "CDE WARNING: renameat '%s' is a relative path and dirfd != AT_FDCWD\n",
            oldpath);
    goto done; // punt early!
  }
  if (!IS_ABSPATH(newpath) && tcp->u_arg[2] != AT_FDCWD) {
    fprintf(stderr,
            "CDE WARNING: renameat '%s' is a relative path and dirfd != AT_FDCWD\n",
            newpath);
    goto done; // punt early!
  }

  if (CDE_exec_mode) {
    modify_syscall_second_and_fourth_args(tcp);
  }

done:
  free(oldpath);
  free(newpath);
}

void CDE_end_file_renameat(struct tcb* tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] END renameat\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    // empty
  }
  else {
    if (tcp->u_rval == 0) {
      char* filename1 = strcpy_from_child(tcp, tcp->u_arg[1]);
      char* redirected_filename1 =
        redirect_filename_into_cderoot(filename1, tcp->current_dir, tcp);
      free(filename1);
      // remove original file from cde-root/
      if (redirected_filename1) {
        unlink(redirected_filename1);
        free(redirected_filename1);
      }

      // copy the destination file into cde-root/
      char* dst_filename = strcpy_from_child(tcp, tcp->u_arg[3]);
      copy_file_into_cde_root(dst_filename, tcp->current_dir);
      free(dst_filename);
    }
  }
}


void CDE_begin_chdir(struct tcb* tcp) {
  CDE_begin_standard_fileop(tcp, "chdir");
}

void CDE_end_fchdir(struct tcb* tcp);

void CDE_end_chdir(struct tcb* tcp) {
  CDE_end_fchdir(tcp); // this will update tcp->current_dir
}

void CDE_end_fchdir(struct tcb* tcp) {
  // only do this on success
  if (tcp->u_rval == 0) {
    // update current_dir

    // A reliable way to get the current directory is using /proc/<pid>/cwd
    char* cwd_symlink_name = format("/proc/%d/cwd", tcp->pid);

    tcp->current_dir[0] = '\0';
    int len = readlink(cwd_symlink_name, tcp->current_dir, MAXPATHLEN);
    assert(tcp->current_dir[0] != '\0');
    assert(len >= 0);
    tcp->current_dir[len] = '\0'; // wow, readlink doesn't put the cap on the end!!!

    free(cwd_symlink_name);


    // now copy into cde-root/ if necessary
    if (!CDE_exec_mode) {
      char* redirected_path =
        redirect_filename_into_cderoot(tcp->current_dir, tcp->current_dir, tcp);
      if (redirected_path) {
        make_mirror_dirs_in_cde_package(tcp->current_dir, 0);
        free(redirected_path);
      }
    }
  }
}


void CDE_begin_mkdir(struct tcb* tcp) {
  CDE_begin_standard_fileop(tcp, "mkdir");
}

void CDE_end_mkdir(struct tcb* tcp, int input_buffer_arg_index) {
  if (CDE_verbose_mode) {
    printf("[%d] END mkdir*\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    // empty
  }
  else {
    // mkdir either when the call succeeds or only fails because the
    // directory already exists
    if ((tcp->u_rval == 0) || (tcp->u_rval == EEXIST)) {
      // sometimes mkdir is called with a BOGUS argument, so silently skip those cases
      char* dirname_arg = strcpy_from_child(tcp, tcp->u_arg[input_buffer_arg_index]);
      char* dirname_abspath = canonicalize_path(dirname_arg, tcp->current_dir);
      make_mirror_dirs_in_cde_package(dirname_abspath, 0);
      free(dirname_abspath);
      free(dirname_arg);
    }
  }
}

// copy-and-paste from mkdir functions above,
// except adjusting for mkdirat signature:
//   int mkdirat(int dirfd, const char *pathname, mode_t mode);
void CDE_begin_mkdirat(struct tcb* tcp) {
  CDE_begin_at_fileop(tcp, "mkdirat");
}

void CDE_end_mkdirat(struct tcb* tcp) {
  CDE_end_mkdir(tcp, 1);
}


void CDE_begin_rmdir(struct tcb* tcp) {
  CDE_begin_standard_fileop(tcp, "rmdir");
}

void CDE_end_rmdir(struct tcb* tcp, int input_buffer_arg_index) {
  if (CDE_verbose_mode) {
    printf("[%d] END rmdir*\n", tcp->pid);
  }

  if (CDE_exec_mode) {
    // empty
  }
  else {
    if (tcp->u_rval == 0) {
      char* dirname_arg = strcpy_from_child(tcp, tcp->u_arg[input_buffer_arg_index]);
      char* redirected_path =
        redirect_filename_into_cderoot(dirname_arg, tcp->current_dir, tcp);
      if (redirected_path) {
        rmdir(redirected_path);
        free(redirected_path);
      }
      free(dirname_arg);
    }
  }
}


void CDE_begin_unlinkat_rmdir(struct tcb* tcp) {
  CDE_begin_at_fileop(tcp, "unlinkat_rmdir");
}

void CDE_end_unlinkat_rmdir(struct tcb* tcp) {
  CDE_end_rmdir(tcp, 1);
}


// from Goanna
#define FILEBACK 8 /* It is OK to use a file backed region. */

// TODO: this is probably very Linux-specific ;)
static void* find_free_addr(int pid, int prot, unsigned long size) {
  FILE *f;
  char filename[20];
  char s[80];
  char r, w, x, p;

  sprintf(filename, "/proc/%d/maps", pid);

  f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Can not find a free address in pid %d: %s\n.", pid, strerror(errno));
  }
  while (fgets(s, sizeof(s), f) != NULL) {
    unsigned long cstart, cend;
    int major, minor;

    sscanf(s, "%lx-%lx %c%c%c%c %*x %x:%x", &cstart, &cend, &r, &w, &x, &p, &major, &minor);

    if (cend - cstart < size) {
      continue;
    }

    if (!(prot & FILEBACK) && (major || minor)) {
      continue;
    }

    if (p != 'p') {
      continue;
    }
    if ((prot & PROT_READ) && (r != 'r')) {
      continue;
    }
    if ((prot & PROT_EXEC) && (x != 'x')) {
      continue;
    }
    if ((prot & PROT_WRITE) && (w != 'w')) {
      continue;
    }
    fclose(f);

    return (void *)cstart;
  }
  fclose(f);

  return NULL;
}


void alloc_tcb_CDE_fields(struct tcb* tcp) {
  tcp->localshm = NULL;
  tcp->childshm = NULL;
  tcp->setting_up_shm = 0;

  if (CDE_exec_mode) {
    key_t key;
    // randomly probe for a valid shm key
    do {
      errno = 0;
      key = rand();
      tcp->shmid = shmget(key, SHARED_PAGE_SIZE, IPC_CREAT|IPC_EXCL|0600);
    } while (tcp->shmid == -1 && errno == EEXIST);

    tcp->localshm = (char*)shmat(tcp->shmid, NULL, 0);

    if ((long)tcp->localshm == -1) {
      perror("shmat");
      exit(1);
    }

    if (shmctl(tcp->shmid, IPC_RMID, NULL) == -1) {
      perror("shmctl(IPC_RMID)");
      exit(1);
    }

    assert(tcp->localshm);
  }

  tcp->current_dir = NULL;
  tcp->p_ignores = NULL;
}

void free_tcb_CDE_fields(struct tcb* tcp) {
  if (tcp->localshm) {
    shmdt(tcp->localshm);
  }
  // need to null out elts in case table entries are recycled
  tcp->localshm = NULL;
  tcp->childshm = NULL;
  tcp->setting_up_shm = 0;
  tcp->p_ignores = NULL;

  if (tcp->current_dir) {
    free(tcp->current_dir);
    tcp->current_dir = NULL;
  }
}


// inject a system call in the child process to tell it to attach our
// shared memory segment, so that it can read modified paths from there
//
// Setup a shared memory region within child process,
// then repeat current system call
//
// WARNING: this code is very tricky and gross!
static void begin_setup_shmat(struct tcb* tcp) {
  assert(tcp->localshm);
  assert(!tcp->childshm); // avoid duplicate calls

  // stash away original registers so that we can restore them later
  struct user_regs_struct cur_regs;
  EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);
  memcpy(&tcp->saved_regs, &cur_regs, sizeof(cur_regs));

#if defined (I386)
  // The return value of shmat (attached address) is actually stored in
  // the child's address space
  tcp->savedaddr = find_free_addr(tcp->pid, PROT_READ|PROT_WRITE, sizeof(int));

  // store *tcp->savedaddr data (in child's address space) so that we can restore it later:
  tcp->savedword = ptrace(PTRACE_PEEKDATA, tcp->pid, tcp->savedaddr, 0);
  EXITIF(errno); // PTRACE_PEEKDATA reports error in errno

  // To make the target process execute a shmat() on 32-bit x86, we need to make
  // it execute the special __NR_ipc syscall with SHMAT as a param:

  /* The shmat call is implemented as a godawful sys_ipc. */
  cur_regs.orig_eax = __NR_ipc;
  /* The parameters are passed in ebx, ecx, edx, esi, edi, and ebp */
  cur_regs.ebx = SHMAT;
  /* The kernel names the rest of these, first, second, third, ptr,
   * and fifth. Only first, second and ptr are used as inputs.  Third
   * is a pointer to the output (unsigned long).
   */
  cur_regs.ecx = tcp->shmid;
  cur_regs.edx = 0; /* shmat flags */
  cur_regs.esi = (long)tcp->savedaddr; /* Pointer to the return value in the
                                          child's address space. */
  cur_regs.edi = (long)NULL; /* We don't use shmat's shmaddr */
  cur_regs.ebp = 0; /* The "fifth" argument is unused. */
#elif defined(X86_64)
  if (IS_32BIT_EMU) {
    // If we're on a 64-bit machine but tracing a 32-bit target process, then we
    // need to make the 32-bit __NR_ipc SHMAT syscall as though we're on a 32-bit
    // machine (see code above), except that we use registers like 'rbx' rather
    // than 'ebx'.  This was VERY SUBTLE AND TRICKY to finally get right!

    // this code is almost exactly copy-and-paste from the I386 section above,
    // except that the register names are the x86-64 versions of the 32-bit regs
    tcp->savedaddr = find_free_addr(tcp->pid, PROT_READ|PROT_WRITE, sizeof(int));
    tcp->savedword = ptrace(PTRACE_PEEKDATA, tcp->pid, tcp->savedaddr, 0);
    EXITIF(errno);

    cur_regs.orig_rax = 117; // 117 is the numerical value of the __NR_ipc macro (not available on 64-bit hosts!)
    cur_regs.rbx = 21;       // 21 is the numerical value of the SHMAT macro (not available on 64-bit hosts!)
    cur_regs.rcx = tcp->shmid;
    cur_regs.rdx = 0;
    cur_regs.rsi = (long)tcp->savedaddr;
    cur_regs.rdi = (long)NULL;
    cur_regs.rbp = 0;
  }
  else {
    // If the target process is 64-bit, then life is good, because
    // there is a direct shmat syscall in x86-64!!!
    cur_regs.orig_rax = __NR_shmat;
    cur_regs.rdi = tcp->shmid;
    cur_regs.rsi = 0;
    cur_regs.rdx = 0;
  }
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

  EXITIF(ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

  tcp->setting_up_shm = 1; // very importante!!!
}

void finish_setup_shmat(struct tcb* tcp) {
  struct user_regs_struct cur_regs;
  EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
  // setup had better been a success!
  assert(cur_regs.orig_eax == __NR_ipc);
  assert(cur_regs.eax == 0);

  // the pointer to the shared memory segment allocated by shmat() is actually
  // located in *tcp->savedaddr (in the child's address space)
  errno = 0;
  tcp->childshm = (void*)ptrace(PTRACE_PEEKDATA, tcp->pid, tcp->savedaddr, 0);
  EXITIF(errno); // PTRACE_PEEKDATA reports error in errno

  // restore original data in child's address space
  EXITIF(ptrace(PTRACE_POKEDATA, tcp->pid, tcp->savedaddr, tcp->savedword));

  tcp->saved_regs.eax = tcp->saved_regs.orig_eax;

  // back up IP so that we can re-execute previous instruction
  // TODO: is the use of 2 specific to 32-bit machines?
  tcp->saved_regs.eip = tcp->saved_regs.eip - 2;
#elif defined(X86_64)
  if (IS_32BIT_EMU) {
    // If we're on a 64-bit machine but tracing a 32-bit target process, then we
    // need to handle the return value of the 32-bit __NR_ipc SHMAT syscall as
    // though we're on a 32-bit machine (see code above).  This was VERY SUBTLE
    // AND TRICKY to finally get right!

    // setup had better been a success!
    assert(cur_regs.orig_rax == 117 /*__NR_ipc*/);
    assert(cur_regs.rax == 0);

    // the pointer to the shared memory segment allocated by shmat() is actually
    // located in *tcp->savedaddr (in the child's address space)
    errno = 0;

    // this is SUPER IMPORTANT ... only keep the 32 least significant bits
    // (mask with 0xffffffff) before storing the pointer in tcp->childshm,
    // since 32-bit processes only have 32-bit addresses, not 64-bit addresses :0
    tcp->childshm = (void*)(ptrace(PTRACE_PEEKDATA, tcp->pid, tcp->savedaddr, 0) & 0xffffffff);
    EXITIF(errno);
    // restore original data in child's address space
    EXITIF(ptrace(PTRACE_POKEDATA, tcp->pid, tcp->savedaddr, tcp->savedword));
  }
  else {
    // If the target process is 64-bit, then life is good, because
    // there is a direct shmat syscall in x86-64!!!
    assert(cur_regs.orig_rax == __NR_shmat);

    // the return value of the direct shmat syscall is in %rax
    tcp->childshm = (void*)cur_regs.rax;
  }

  // the code below is identical regardless of whether the target process is
  // 32-bit or 64-bit (on a 64-bit host)
  tcp->saved_regs.rax = tcp->saved_regs.orig_rax;

  // back up IP so that we can re-execute previous instruction
  // ... wow, apparently the -2 offset works for 64-bit as well :)
  tcp->saved_regs.rip = tcp->saved_regs.rip - 2;
#else
  #error "Unknown architecture (not I386 or X86_64)"
#endif

  EXITIF(ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&tcp->saved_regs) < 0);

  assert(tcp->childshm);

  tcp->setting_up_shm = 0; // very importante!!!
}


// copy src into dst, redirecting it into cde-root/ if necessary
// based on cde_starting_pwd
//
// dst should be big enough to hold a full path
void strcpy_redirected_cderoot(char* dst, char* src) {
  assert(CDE_exec_mode);
  // use cde_starting_pwd (TODO: is that correct?)
  char* redirected_src = redirect_filename_into_cderoot(src, cde_starting_pwd, NULL);
  if (redirected_src) {
    strcpy(dst, redirected_src);
    free(redirected_src);
  }
  else {
    strcpy(dst, src);
  }
}

// malloc a new string from child, and return NULL on failure
static char* strcpy_from_child_or_null(struct tcb* tcp, long addr) {
  char path[MAXPATHLEN];
  if (umovestr(tcp, addr, sizeof path, path) < 0) {
    return NULL;
  }

  return strdup(path);
}

// aborts the program if there's an error in strcpy_from_child_or_null
static char* strcpy_from_child(struct tcb* tcp, long addr) {
  char* ret = strcpy_from_child_or_null(tcp, addr);
  EXITIF(ret == NULL);
  return ret;
}


// adapted from the Goanna project by Spillane et al.
// dst_in_child is a pointer in the child's address space
static void memcpy_to_child(int pid, char* dst_child, char* src, int size) {
  while (size >= sizeof(int)) {
    long w = *((long*)src);
    EXITIF(ptrace(PTRACE_POKEDATA, pid, dst_child, (long)w) < 0);
    size -= sizeof(int);
    dst_child = (char*)dst_child + sizeof(int);
    src = (char*)src + sizeof(int);
  }

  /* Cleanup the last little bit. */
  if (size) {
    union {
        long l;
        char c[4];
    } dw, sw;
    errno = 0;
    dw.l = ptrace(PTRACE_PEEKDATA, pid, dst_child, 0);
    EXITIF(errno);
    sw.l = *((long*)src);

    /* Little endian sucks. */
    dw.c[0] = sw.c[0];
    if (size >= 2)
      dw.c[1] = sw.c[1];
    if (size >= 3)
      dw.c[2] = sw.c[2];
	  assert(size < 4);

    EXITIF(ptrace(PTRACE_POKEDATA, pid, dst_child, dw.l) < 0);
  }
}


// TODO: do we still need to keep track of tcp->child_current_pwd
// if we can just directly access it using /proc/<pid>/cwd ???
void CDE_end_getcwd(struct tcb* tcp) {
  if (!syserror(tcp)) {
    if (CDE_exec_mode) {
      char* sandboxed_pwd = extract_sandboxed_pwd(tcp->current_dir, tcp);
      memcpy_to_child(tcp->pid, (char*)tcp->u_arg[0],
                      sandboxed_pwd, strlen(sandboxed_pwd) + 1);

      // for debugging
      //char* tmp = strcpy_from_child(tcp, tcp->u_arg[0]);
      //printf("[%d] CDE_end_getcwd spoofed: %s\n", tcp->pid, tmp);
      //free(tmp);
    }
    else {
      char* tmp = strcpy_from_child(tcp, tcp->u_arg[0]);
      strcpy(tcp->current_dir, tmp);
      free(tmp);
      //printf("[%d] CDE_end_getcwd: %s\n", tcp->pid, tcp->current_dir);
    }
  }
}


// path_envvar is $PATH.  Iterate through all entries and if any of them
// are symlinks, then create their corresponding entries in cde-root/.
// This takes care of cases where, say, /bin is actually a symlink to
// another directory like /KNOPPIX/bin.  We need to create a symlink
// 'bin' in cde-root/ and point it to ./KNOPPIX/bin
//
// DO THIS AT THE VERY BEGINNING OF EXECUTION!
static void CDE_create_path_symlink_dirs() {
  char *p;
  int m, n;
  struct stat st;
  char tmp_buf[MAXPATHLEN];

  for (p = getenv("PATH"); p && *p; p += m) {
    if (strchr(p, ':')) {
      n = strchr(p, ':') - p;
      m = n + 1;
    }
    else {
      m = n = strlen(p);
    }

    strncpy(tmp_buf, p, n);
    tmp_buf[n] = '\0';

    // this will NOT follow the symlink ...
    if (lstat(tmp_buf, &st) == 0) {
      char is_symlink = S_ISLNK(st.st_mode);
      if (is_symlink) {
        char* tmp = strdup(tmp_buf);
        copy_file_into_cde_root(tmp, cde_starting_pwd);
        free(tmp);
      }
    }
  }

  // also, this is hacky, but also check /usr/lib to see
  // whether it's a symlink.  ld-linux.so.2 will likely try to look
  // for libraries in those places, but they're not in any convenient
  // environment variable
  //
  // note that the other 2 directories that ld-linux.so.2 usually
  // tries to look for libs in, /bin and /lib, will be taken care of by
  // CDE_create_toplevel_symlink_dirs()
  strcpy(tmp_buf, "/usr/lib");
  // this will NOT follow the symlink ...
  if (lstat(tmp_buf, &st) == 0) {
    char is_symlink = S_ISLNK(st.st_mode);
    if (is_symlink) {
      char* tmp = strdup(tmp_buf);
      copy_file_into_cde_root(tmp, cde_starting_pwd);
      free(tmp);
    }
  }
}

// scan through all files at top-level root directory ('/') and find if
// any of them are symlinks to DIRECTORIES.  if so, then copy the symlinks
// and their targets into CDE_ROOT_DIR, so that we can faithfully mirror the
// original filesystem (at least w.r.t. toplevel symlinks).
//
// this is necessary to ensure proper functioning
// on filesystems that have symlinks at the top level.  e.g., on Knoppix
// 2006-06-01 LiveCD, here is the top-level filesystem structure:
/*
  /
    UNIONFS/
      bin
      boot
      etc
      ...
    ramdisk/
      home/
    bin  --> /UNIONFS/bin   (symlink!)
    boot --> /UNIONFS/boot  (symlink!)
    home --> /ramdisk/home  (symlink)
    etc  --> /UNIONFS/etc   (symlink!)
    ...
    usr --> /UNIONFS/usr
*/
static void CDE_create_toplevel_symlink_dirs() {
  DIR* dp = opendir("/");
  assert(dp);
  struct dirent *ep;
  while ((ep = readdir(dp))) {
    char* toplevel_abspath = format("/%s", ep->d_name); // make into abspath
    struct stat st;
    if (lstat(toplevel_abspath, &st) == 0) {
      char is_symlink = S_ISLNK(st.st_mode);
      if (is_symlink) {
        struct stat real_st;
        // only do this for top-level symlinks to DIRECTORIES
        if ((stat(toplevel_abspath, &real_st) == 0) &&
            S_ISDIR(real_st.st_mode)) {
          copy_file_into_cde_root(toplevel_abspath, cde_starting_pwd);
        }
      }
    }
    free(toplevel_abspath);
  }
  closedir(dp);
}


void CDE_init_tcb_dir_fields(struct tcb* tcp) {
  // malloc new entries, and then decide whether to inherit from parent
  // process entry or directly initialize
  assert(!tcp->current_dir);
  tcp->current_dir = malloc(MAXPATHLEN); // big boy!

  // if parent exists, then its fields MUST be legit, so grab them
  if (tcp->parent) {
    assert(tcp->parent->current_dir);
    strcpy(tcp->current_dir, tcp->parent->current_dir);
    //printf("inherited %s [%d]\n", tcp->current_dir, tcp->pid);

    // inherit from parent since you're executing the same program after
    // forking (at least until you do an exec)
    tcp->p_ignores = tcp->parent->p_ignores;
  }
  else {
    // otherwise create fresh fields derived from master (cde) process
    getcwd(tcp->current_dir, MAXPATHLEN);
    //printf("fresh %s [%d]\n", tcp->current_dir, tcp->pid);
  }


  // it's possible that tcp->perceived_program_fullpath has already been
  // set, and if so, don't mess with it.  only inherit from parent if it
  // hasn't been set yet (TODO: I don't fully understand the rationale
  // for this, but it seems to work in practice so far)
  if (!tcp->perceived_program_fullpath && tcp->parent) {
    // aliased, so don't mutate or free
    tcp->perceived_program_fullpath = tcp->parent->perceived_program_fullpath;
  }
}


// find the absolute path to the cde-root/ directory, since that
// will be where our fake filesystem starts.  e.g., if our real pwd is:
//   /home/bob/cde-package/cde-root/home/alice/cool-experiment
// then the pseudo_root_dir is:
//   /home/bob/cde-package/cde-root
//
// if we're running cde-exec from outside of a cde-root/ directory,
// then try to find the cde-root/ corresponding to the location of the
// cde-exec executable
void CDE_init_pseudo_root_dir() {
  assert(CDE_exec_mode);

  struct path* p = new_path_from_abspath(cde_starting_pwd);
  assert(p->depth > 0);
  int i;
  int found_index = -1;
  for (i = 1; i <= p->depth; i++) {
    char* component = get_path_component(p, i);
    if (strcmp(component, CDE_ROOT_NAME) == 0) {
      // flag an error if there is more than one cde-root directory, since
      // we don't support NESTED cde packages o.O
      if (found_index >= 0) {
        fprintf(stderr, "Error: More than one cde-root/ directory found in pwd:\n  '%s'\n",
                cde_starting_pwd);
        exit(1);
      }

      found_index = i;
      // keep searching in case there are duplicates, in which case the
      // above assertion will fail
    }
  }

  if (found_index < 0) {
    // if we can't find 'cde-root' in cde_starting_pwd, then we must
    // be executing cde-exec from OUTSIDE of a repository, so set
    // cde_pseudo_root_dir to:
    //   dirname(readlink("/proc/self/exe")) + "/cde-root"
    char proc_self_exe[MAXPATHLEN];
    proc_self_exe[0] = '\0';
    int len = readlink("/proc/self/exe",
                       proc_self_exe, sizeof proc_self_exe);
    assert(proc_self_exe[0] != '\0');
    assert(len >= 0);
    proc_self_exe[len] = '\0'; // wow, readlink doesn't put cap on the end!

    char* toplevel_cde_root_path =
      format("%s/cde-root", dirname(proc_self_exe));

    strcpy(cde_pseudo_root_dir, toplevel_cde_root_path);

    free(toplevel_cde_root_path);

    cde_exec_from_outside_cderoot = 1;
  }
  else {
    // normal case --- we're currently within a cde-root/ directory, so
    // set that as cde_pseudo_root_dir
    char* tmp = path2str(p, found_index);
    strcpy(cde_pseudo_root_dir, tmp);
    free(tmp);
  }

  delete_path(p);
}


// pgbovine - do all CDE initialization here after command-line options
// have been processed (argv[optind] is the name of the target program)
void CDE_init(char** argv, int optind) {
  // pgbovine - initialize this before doing anything else!
  getcwd(cde_starting_pwd, sizeof cde_starting_pwd);


  // suppress (most) okapi warnings to prevent terminal noise
  extern char OKAPI_VERBOSE;
  OKAPI_VERBOSE = 0;

  // pgbovine - allow most promiscuous permissions for new files/directories
  umask(0000);


  if (CDE_exec_mode) {
    // must do this before running CDE_init_options()
    CDE_init_pseudo_root_dir();

    if (CDE_exec_streaming_mode) {
      char* tmp = strdup(cde_pseudo_root_dir);
      tmp[strlen(tmp) - strlen(CDE_ROOT_NAME)] = '\0';
      cde_remote_root_dir = format("%scde-remote-root", tmp);
      free(tmp);

      struct stat remote_root_stat;
      if ((stat(cde_remote_root_dir, &remote_root_stat) != 0) ||
          (!S_ISDIR(remote_root_stat.st_mode))) {
        fprintf(stderr, "Fatal error: Running in -s mode but '%s' directory does not exist\n",
                cde_remote_root_dir);
        exit(1);
      }

      // initialize trie
      cached_files_trie = TrieNew();

      char* p = format("%s/../locally-cached-files.txt", cde_pseudo_root_dir);
      cached_files_fp = fopen(p, "r");

      if (cached_files_fp) {
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&line, &len, cached_files_fp)) != -1) {
          assert(line[read-1] == '\n');
          line[read-1] = '\0'; // strip of trailing newline
          if (line[0] != '\0') {
            // pre-seed cached_files_trie:
            TrieInsert(cached_files_trie, line);
          }
        }
        fclose(cached_files_fp);
      }

      // always open in append mode so that we can be ready to add more
      // entries on subsequent runs ...
      cached_files_fp = fopen(p, "a");

      free(p);
    }

  }
  else {
    if (!CDE_PACKAGE_DIR) { // if it hasn't been set by the '-o' option, set to a default
      CDE_PACKAGE_DIR = (char*)"cde-package";
    }

    // make this an absolute path!
    CDE_PACKAGE_DIR = canonicalize_path(CDE_PACKAGE_DIR, cde_starting_pwd);
    CDE_ROOT_DIR = format("%s/%s", CDE_PACKAGE_DIR, CDE_ROOT_NAME);
    assert(IS_ABSPATH(CDE_ROOT_DIR));

    mkdir(CDE_PACKAGE_DIR, 0777);
    mkdir(CDE_ROOT_DIR, 0777);


    // if we can't even create CDE_ROOT_DIR, then abort with a failure
    struct stat cde_rootdir_stat;
    if (stat(CDE_ROOT_DIR, &cde_rootdir_stat)) {
      fprintf(stderr, "Error: Cannot create CDE root directory at \"%s\"\n", CDE_ROOT_DIR);
      exit(1);
    }


    // collect uname information in CDE_PACKAGE_DIR/cde.uname
    struct utsname uname_info;
    if (uname(&uname_info) >= 0) {
      char* fn = format("%s/cde.uname", CDE_PACKAGE_DIR);
      FILE* uname_f = fopen(fn, "w");
      free(fn);
      if (uname_f) {
        fprintf(uname_f, "uname: '%s' '%s' '%s' '%s'\n",
                          uname_info.sysname,
                          uname_info.release,
                          uname_info.version,
                          uname_info.machine);
        fclose(uname_f);
      }
    }

    // if cde.options doesn't yet exist, create it in pwd and seed it
    // with default values that are useful to ignore in practice
    //
    // do this BEFORE CDE_init_options() so that we pick up those
    // ignored values
    struct stat cde_options_stat;
    if (stat("cde.options", &cde_options_stat)) {
      FILE* f = fopen("cde.options", "w");

      fputs(CDE_OPTIONS_VERSION_NUM, f);
      fputs(" (do not alter this first line!)\n", f);

      // /dev, /proc, and /sys are special system directories with fake files
      //
      // some sub-directories within /var contains 'volatile' temp files
      // that change when system is running normally
      //
      // (Note that it's a bit too much to simply ignore all of /var,
      // since files in dirs like /var/lib might be required - e.g., see
      // gnome-sudoku example)
      //
      // $HOME/.Xauthority is used for X11 authentication via ssh, so we need to
      // use the REAL version and not the one in cde-root/
      //
      // ignore "/tmp" and "/tmp/*" since programs often put lots of
      // session-specific stuff into /tmp so DO NOT track files within
      // there, or else you will risk severely 'overfitting' and ruining
      // portability across machines.  it's safe to assume that all Linux
      // distros have a /tmp directory that anybody can write into
      fputs("\n# These directories often contain pseudo-files that shouldn't be tracked\n", f);
      fputs("ignore_prefix=/dev/\n", f);
      fputs("ignore_exact=/dev\n", f);
      fputs("ignore_prefix=/proc/\n", f);
      fputs("ignore_exact=/proc\n", f);
      fputs("ignore_prefix=/sys/\n", f);
      fputs("ignore_exact=/sys\n", f);
      fputs("ignore_prefix=/var/cache/\n", f);
      fputs("ignore_prefix=/var/lock/\n", f);
      fputs("ignore_prefix=/var/log/\n", f);
      fputs("ignore_prefix=/var/run/\n", f);
      fputs("ignore_prefix=/var/tmp/\n", f);
      fputs("ignore_prefix=/tmp/\n", f);
      fputs("ignore_exact=/tmp\n", f);

      fputs("\n# un-comment the entries below if you think they might help your app:\n", f);
      fputs("#ignore_exact=/etc/ld.so.cache\n", f);
      fputs("#ignore_exact=/etc/ld.so.preload\n", f);
      fputs("#ignore_exact=/etc/ld.so.nohwcap\n", f);

      fputs("\n# Ignore .Xauthority to allow X Windows programs to work\n", f);
      fputs("ignore_substr=.Xauthority\n", f);

      // we gotta ignore /etc/resolv.conf or else Google Earth can't
      // access the network when on another machine, so it won't work
      // (and I think other network-facing apps might not work either!)
      fputs("\n# Ignore so that networking can work properly\n", f);
      fputs("ignore_exact=/etc/resolv.conf\n", f);

      fputs("# These files might be useful to ignore along with /etc/resolv.conf\n", f);
      fputs("# (un-comment if you want to try them)\n", f);
      fputs("#ignore_exact=/etc/host.conf\n", f);
      fputs("#ignore_exact=/etc/hosts\n", f);
      fputs("#ignore_exact=/etc/nsswitch.conf\n", f);
      fputs("#ignore_exact=/etc/gai.conf\n", f);

      // ewencp also suggests looking into ignoring these other
      // networking-related files:
      /* Hmm, good point. There's probably lots -- if you're trying to
         run a server, /etc/hostname, /etc/hosts.allow and
         /etc/hosts.deny could all be problematic.  /etc/hosts could be
         a problem for client or server, although its unusual to have
         much in there. One way it could definitely be a problem is if
         the hostname is in /etc/hosts and you want to use it as a
         server, e.g. I run on my machine (ahoy) the server and client,
         which appears in /etc/hosts, and then when cde-exec runs it
         ends up returning 127.0.0.1.  But for all of these, I actually
         don't know when the file gets read, so I'm not certain any of
         them are really a problem. */

      fputs("\n# Access the target machine's password files:\n", f);
      fputs("# (some programs like texmacs need these lines to be commented-out,\n", f);
      fputs("#  since they try to use home directory paths within the passwd file,\n", f);
      fputs("#  and those paths might not exist within the package.)\n", f);
      fputs("ignore_prefix=/etc/passwd\n", f);
      fputs("ignore_prefix=/etc/shadow\n", f);


      fputs("\n# These environment vars might lead to 'overfitting' and hinder portability\n", f);
      fputs("ignore_environment_var=DBUS_SESSION_BUS_ADDRESS\n", f);
      fputs("ignore_environment_var=ORBIT_SOCKETDIR\n", f);
      fputs("ignore_environment_var=SESSION_MANAGER\n", f);
      fputs("ignore_environment_var=XAUTHORITY\n", f);
      fputs("ignore_environment_var=DISPLAY\n", f);
     
      fclose(f);
    }
  }


  // do this AFTER creating cde.options
  CDE_init_options();


  if (CDE_exec_mode) {
    CDE_load_environment_vars();
  }
  else {
    // pgbovine - copy 'cde' executable to CDE_PACKAGE_DIR and rename
    // it 'cde-exec', so that it can be included in the executable
    //
    // use /proc/self/exe since argv[0] might be simply 'cde'
    // (if the cde binary is in $PATH and we're invoking it only by its name)
    char* fn = format("%s/cde-exec", CDE_PACKAGE_DIR);
    copy_file((char*)"/proc/self/exe", fn, 0777);
    free(fn);

    CDE_create_convenience_scripts(argv, optind);


    // make a cde.log file that contains commands to reproduce original
    // run within cde-package
    struct stat tmp;
    FILE* log_f;
    char* log_filename = format("%s/cde.log", CDE_PACKAGE_DIR);
    if (stat(log_filename, &tmp)) {
      log_f = fopen(log_filename, "w");
      fprintf(log_f, "cd '" CDE_ROOT_NAME "%s'", cde_starting_pwd);
      fputc('\n', log_f);
    }
    else {
      log_f = fopen(log_filename, "a");
    }
    free(log_filename);

    fprintf(log_f, "'./%s.cde'", basename(argv[optind]));
    int i;
    for (i = optind + 1; argv[i] != NULL; i++) {
      fprintf(log_f, " '%s'", argv[i]); // add quotes for accuracy
    }
    fputc('\n', log_f);
    fclose(log_f);

    CDE_create_path_symlink_dirs();

    CDE_create_toplevel_symlink_dirs();


    // copy /proc/self/environ to capture the FULL set of environment vars
    char* fullenviron_fn = format("%s/cde.full-environment", CDE_PACKAGE_DIR);
    copy_file((char*)"/proc/self/environ", fullenviron_fn, 0666);
    free(fullenviron_fn);
  }


}


// create a '.cde' version of the target program inside the corresponding
// location of cde_starting_pwd within CDE_ROOT_DIR, which is a
// shell script that invokes it using cde-exec
//
// also, if target_program_fullpath is only a program name
// (without any '/' chars in it, then also create a convenience script
// at the top level of the package)
//
// argv[optind] is the target program's name
static void CDE_create_convenience_scripts(char** argv, int optind) {
  assert(!CDE_exec_mode);
  char* target_program_fullpath = argv[optind];

  // only take the basename to construct cde_script_name,
  // since target_program_fullpath could be a relative path like '../python'
  char* cde_script_name = format("%s.cde", basename(target_program_fullpath));
  char* progname_redirected =
    redirect_filename_into_cderoot(cde_script_name, cde_starting_pwd, NULL);

  if (progname_redirected) {
    // make sure directory exists :)
    make_mirror_dirs_in_cde_package(cde_starting_pwd, 0);

    // this is sort of tricky.  we need to insert in a bunch of ../ so
    // that we can find cde-exec, which is right in the cde-package directory
    struct path* p = new_path_from_abspath(cde_starting_pwd);
    char dot_dots[MAXPATHLEN];
    assert(p->depth > 0);
    strcpy(dot_dots, "..");
    int i;
    for (i = 1; i <= p->depth; i++) {
      strcat(dot_dots, "/..");
    }
    delete_path(p);

    FILE* f = fopen(progname_redirected, "w");
    fprintf(f, "#!/bin/sh\n");
    fprintf(f, "%s/cde-exec", dot_dots);
    // include original command-line options
    for (i = 1; i < optind; i++) {
      fprintf(f, " '%s'", argv[i]);
    }
    // double quotes seem to work well for making $@ more accurate
    fprintf(f, " '%s' \"$@\"\n", target_program_fullpath);
    fclose(f);

    chmod(progname_redirected, 0777); // now make the script executable

    free(progname_redirected);
  }

  if (!strchr(target_program_fullpath, '/')) {
    char* toplevel_script_name = format("%s/%s", CDE_PACKAGE_DIR, cde_script_name);
    FILE* f = fopen(toplevel_script_name, "w");

    // Thanks to probono@puredarwin.org for the following more robust
    // start-up script idea, which creates a program that can be
    // double-clicked and run from anywhere.
    fprintf(f, "#!/bin/sh\n");
    fprintf(f, "HERE=\"$(dirname \"$(readlink -f \"${0}\")\")\"\n");
    fprintf(f, "cd \"$HERE/cde-root\" && ../cde-exec");

    // include original command-line options
    int i;
    for (i = 1; i < optind; i++) {
      fprintf(f, " '%s'", argv[i]);
    }
    // double quotes seem to work well for make $@ more accurate
    fprintf(f, " '%s' \"$@\"\n", target_program_fullpath);

    fclose(f);
    chmod(toplevel_script_name, 0777); // now make the script executable
    free(toplevel_script_name);
  }

  free(cde_script_name);
}


static void _add_to_array_internal(char** my_array, int* p_len, char* p, char* array_name) {
  assert(my_array[*p_len] == NULL);
  my_array[*p_len] = strdup(p);


  if (CDE_verbose_mode) {
    printf("%s[%d] = '%s'\n", array_name, *p_len, my_array[*p_len]);
  }

  (*p_len)++;

  if (*p_len >= 100) {
    fprintf(stderr, "Fatal error: more than 100 entries in %s\n", array_name);
    exit(1);
  }
}

void CDE_add_ignore_exact_path(char* p) {
  _add_to_array_internal(ignore_exact_paths, &ignore_exact_paths_ind, p, (char*)"ignore_exact_paths");
}

void CDE_add_ignore_prefix_path(char* p) {
  _add_to_array_internal(ignore_prefix_paths, &ignore_prefix_paths_ind, p, (char*)"ignore_prefix_paths");
}

void CDE_add_ignore_substr_path(char* p) {
  _add_to_array_internal(ignore_substr_paths, &ignore_substr_paths_ind, p, (char*)"ignore_substr_paths");
}

void CDE_add_redirect_exact_path(char* p) {
  _add_to_array_internal(redirect_exact_paths, &redirect_exact_paths_ind, p, (char*)"redirect_exact_paths");
}

void CDE_add_redirect_prefix_path(char* p) {
  _add_to_array_internal(redirect_prefix_paths, &redirect_prefix_paths_ind, p, (char*)"redirect_prefix_paths");
}

void CDE_add_redirect_substr_path(char* p) {
  _add_to_array_internal(redirect_substr_paths, &redirect_substr_paths_ind, p, (char*)"redirect_substr_paths");
}

void CDE_add_ignore_envvar(char* p) {
  _add_to_array_internal(ignore_envvars, &ignore_envvars_ind, p, (char*)"ignore_envvars");
}


// call this at the VERY BEGINNING of execution, so that ignore paths can be
// specified on the command line (e.g., using the '-i' and '-p' options)
void CDE_clear_options_arrays() {
  memset(ignore_exact_paths,    0, sizeof(ignore_exact_paths));
  memset(ignore_prefix_paths,   0, sizeof(ignore_prefix_paths));
  memset(ignore_substr_paths,   0, sizeof(ignore_substr_paths));
  memset(redirect_exact_paths,  0, sizeof(redirect_exact_paths));
  memset(redirect_prefix_paths, 0, sizeof(redirect_prefix_paths));
  memset(redirect_substr_paths, 0, sizeof(redirect_substr_paths));
  memset(ignore_envvars,        0, sizeof(ignore_envvars));
  memset(process_ignores,       0, sizeof(process_ignores));


  ignore_exact_paths_ind = 0;
  ignore_prefix_paths_ind = 0;
  ignore_substr_paths_ind = 0;
  redirect_exact_paths_ind = 0;
  redirect_prefix_paths_ind = 0;
  redirect_substr_paths_ind = 0;
  ignore_envvars_ind = 0;
  process_ignores_ind = 0;
}


// initialize arrays based on the cde.options file, which has the grammar:
//
// ignore_exact=<exact path to ignore>
// ignore_prefix=<path prefix to ignore>
// ignore_substr=<path substring to ignore>
// redirect_exact=<exact path to allow>
// redirect_prefix=<path prefix to allow>
// redirect_substr=<path substring to allow>
// ignore_environment_var=<environment variable to ignore>
//
// On 2011-06-22, added support for process-specific ignores, with the following syntax:
// ignore_process=<exact path to ignore>
// {
//   process_ignore_prefix=<path prefix to ignore for the given process>
// }
static void CDE_init_options() {
  // Pre-req: CDE_clear_options_arrays() has already been called!

  char in_braces = false;

  FILE* f = NULL;

  if (CDE_exec_mode) {
    // look for a cde.options file in the package

    // you must run this AFTER running CDE_init_pseudo_root_dir()
    assert(*cde_pseudo_root_dir);
    char* options_file = format("%s/../cde.options", cde_pseudo_root_dir);
    f = fopen(options_file, "r");
    if (!f) {
      fprintf(stderr, "Fatal error: missing cde.options file\n");
      fprintf(stderr, "(trying to locate file at %s)\n", options_file);
      exit(1);
    }
    free(options_file);
  }
  else {
    // look for a cde.options file in pwd
    f = fopen("cde.options", "r");

    // if found, copy it into the package
    if (f) {
      char* fn = format("%s/cde.options", CDE_PACKAGE_DIR);
      copy_file((char*)"cde.options", fn, 0666);
      free(fn);
    }
    else {
      fprintf(stderr, "Fatal error: missing cde.options file\n");
      exit(1);
    }
  }

  char is_first_line = 1;

  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, f)) != -1) {
    assert(line[read-1] == '\n');
    line[read-1] = '\0'; // strip of trailing newline

    // strip off leading and trailing spaces
    while (*line && isspace(*line)) {
      line++;
    }

    int last = strlen(line) - 1;
    while (last >= 0 && isspace(line[last])) {
      line[last] = '\0';
      last--;
    }

    // make sure there's an appropriate version number on first line
    if (is_first_line) {
      if (strncmp(line, CDE_OPTIONS_VERSION_NUM, strlen(CDE_OPTIONS_VERSION_NUM)) != 0) {
        fprintf(stderr, "Error: cde.options file incompatible with this version of cde ('%s')\n",
                CDE_OPTIONS_VERSION_NUM);
        exit(1);
      }
      is_first_line = 0;
      continue;
    }

    // ignore blank or comment lines
    if (line[0] == '\0' || line[0] == '#') {
      continue;
    }

    // for process_ignore_prefix directives
    if (line[0] == '{') {
      assert(process_ignores_ind > 0); // ignore_process must've come first!
      in_braces = 1;
      continue;
    }
    else if (line[0] == '}') {
      in_braces = 0;
      continue;
    }

    char* p;
    char is_first_token = 1;
    char set_id = -1;

    for (p = strtok(line, "="); p; p = strtok(NULL, "=")) {
      if (is_first_token) {
        if (strcmp(p, "ignore_exact") == 0) {
          set_id = 1;
        }
        else if (strcmp(p, "ignore_prefix") == 0) {
          set_id = 2;
        }
        else if (strcmp(p, "ignore_environment_var") == 0) {
          set_id = 3;
        }
        else if (strcmp(p, "redirect_exact") == 0) {
          set_id = 4;
        }
        else if (strcmp(p, "redirect_prefix") == 0) {
          set_id = 5;
        }
        else if (strcmp(p, "ignore_substr") == 0) {
          set_id = 6;
        }
        else if (strcmp(p, "redirect_substr") == 0) {
          set_id = 7;
        }
        else if (strcmp(p, "ignore_process") == 0) {
          set_id = 8;
        }
        else if (strcmp(p, "process_ignore_prefix") == 0) {
          if (!in_braces) {
            fprintf(stderr, "Fatal error in cde.options: 'process_ignore_prefix' must be enclosed in { } after an 'ignore_process' directive\n");
            exit(1);
          }
          set_id = 9;
        }
        else {
          fprintf(stderr, "Fatal error in cde.options: unrecognized token '%s'\n", p);
          exit(1);
        }

        if (in_braces && set_id != 9) {
          fprintf(stderr, "Fatal error in cde.options: Only 'process_ignore_prefix' is allowed within { } after an 'ignore_process' directive\n");
          exit(1);
        }

        is_first_token = 0;
      }
      else {
        struct PI* cur = NULL;

        switch (set_id) {
          case 1:
            CDE_add_ignore_exact_path(p);
            break;
          case 2:
            CDE_add_ignore_prefix_path(p);
            break;
          case 3:
            CDE_add_ignore_envvar(p);
            break;
          case 4:
            CDE_add_redirect_exact_path(p);
            break;
          case 5:
            CDE_add_redirect_prefix_path(p);
            break;
          case 6:
            CDE_add_ignore_substr_path(p);
            break;
          case 7:
            CDE_add_redirect_substr_path(p);
            break;
          case 8:
            assert(process_ignores[process_ignores_ind].process_name == NULL);
            process_ignores[process_ignores_ind].process_name = strdup(p);
            process_ignores[process_ignores_ind].process_ignore_prefix_paths_ind = 0;

            // debug printf
            //fprintf(stderr, "process_ignores[%d] = '%s'\n",
            //        process_ignores_ind, process_ignores[process_ignores_ind].process_name);

            process_ignores_ind++;
            if (process_ignores_ind >= 50) {
              fprintf(stderr, "Fatal error in cde.options: more than 50 'ignore_process' entries\n");
              exit(1);
            }
            break;
          case 9:
            assert(process_ignores_ind > 0);
            // attach to the LATEST element in process_ignores
            cur = &process_ignores[process_ignores_ind-1];
            assert(cur->process_name);
            cur->process_ignore_prefix_paths[cur->process_ignore_prefix_paths_ind] = strdup(p);

            // debug printf
            //fprintf(stderr, "process_ignores[%s][%d] = '%s'\n",
            //        cur->process_name,
            //        cur->process_ignore_prefix_paths_ind,
            //        cur->process_ignore_prefix_paths[cur->process_ignore_prefix_paths_ind]);

            cur->process_ignore_prefix_paths_ind++;
            if (cur->process_ignore_prefix_paths_ind >= 20) {
              fprintf(stderr, "Fatal error in cde.options: more than 20 'process_ignore_prefix' entries\n");
              exit(1);
            }
            break;
          default:
            assert(0);
        }

        break;
      }
    }
  }

  fclose(f);

  cde_options_initialized = 1;
}


static void CDE_load_environment_vars() {
  static char cde_full_environment_abspath[MAXPATHLEN];
  strcpy(cde_full_environment_abspath, cde_pseudo_root_dir);
  strcat(cde_full_environment_abspath, "/../cde.full-environment");

  struct stat env_file_stat;
  if (stat(cde_full_environment_abspath, &env_file_stat)) {
    perror(cde_full_environment_abspath);
    exit(1);
  }

  int full_environment_fd = open(cde_full_environment_abspath, O_RDONLY);

  void* environ_start =
    (char*)mmap(0, env_file_stat.st_size, PROT_READ, MAP_PRIVATE, full_environment_fd, 0);

  char* environ_str = (char*)environ_start;
  while (*environ_str) {
    int environ_strlen = strlen(environ_str);

    // format: "name=value"
    // note that 'value' might itself contain '=' characters,
    // so only split on the FIRST '='

    char* cur = strdup(environ_str); // strtok needs to mutate
    char* name = NULL;
    char* val = NULL;

    int count = 0;
    char* p;
    int start_index_of_value = 0;

    // strtok is so dumb!!!  need to munch through the entire string
    // before it restores the string to its original value
    for (p = strtok(cur, "="); p; p = strtok(NULL, "=")) {
      if (count == 0) {
        name = strdup(p);
      }
      else if (count == 1) {
        start_index_of_value = (p - cur);
      }

      count++;
    }

    if (start_index_of_value) {
      val = strdup(environ_str + start_index_of_value);
    }

    // make sure we're not ignoring this environment var:
    int i;
    int ignore_me = 0;
    for (i = 0; i < ignore_envvars_ind; i++) {
      if (strcmp(name, ignore_envvars[i]) == 0) {
        ignore_me = 1;
        break;
      }
    }

    // ignore an invalid variable with an empty name or a name
    // that's simply a newline character (some files have a trailing
    // newline, which strtok picks up, ugh):
    if (!name || (strcmp(name, "\n") == 0)) {
      ignore_me = 1;
    }

    if (!ignore_me) {
      // subtle ... if val is NULL, then we should call setenv() with
      // an empty string as val, NOT a NULL, since calling it with a
      // NULL parameter will cause it to DELETE the environment
      // variable, not set it to ""
      if (val) {
        setenv(name, val, 1);
      }
      else {
        setenv(name, "", 1);
      }
    }
    else {
      if (CDE_verbose_mode) {
        printf("ignored envvar '%s' => '%s'\n", name, val);
      }
    }

    if (name) free(name);
    if (val) free(val);
    free(cur);

    // every string in cde_full_environment_abspath is
    // null-terminated, so this advances to the next string
    environ_str += (environ_strlen + 1);
  }

  munmap(environ_start, env_file_stat.st_size);
  close(full_environment_fd);
}


// if we're running in CDE_exec_mode, redirect path argument for bind()
// and connect() into cde-root sandbox
void CDE_begin_socket_bind_or_connect(struct tcb *tcp) {
  if (CDE_verbose_mode) {
    printf("[%d] BEGIN socket bind/connect\n", tcp->pid);
  }

  // only do this redirection in CDE_exec_mode
  if (!CDE_exec_mode) {
    return;
  }

  // code adapted from printsock in strace-4.5.20/net.c
  long addr = tcp->u_arg[1];
  int addrlen = tcp->u_arg[2];

  union {
    char pad[128];
    struct sockaddr sa;
    struct sockaddr_un sau;
  } addrbuf;

  if (addr == 0) {
    return;
  }

  if (addrlen < 2 || addrlen > sizeof(addrbuf)) {
    addrlen = sizeof(addrbuf);
  }

  memset(&addrbuf, 0, sizeof(addrbuf));
  if (umoven(tcp, addr, addrlen, addrbuf.pad) < 0) {
    return;
  }
  addrbuf.pad[sizeof(addrbuf.pad) - 1] = '\0';

  /* AF_FILE is also a synonym for AF_UNIX */
  if (addrbuf.sa.sa_family == AF_UNIX) {
    if (addrlen > 2 && addrbuf.sau.sun_path[0]) {
      //tprintf("path=");

      // addr + sizeof(addrbuf.sau.sun_family) is the location of the real path
      char* original_path = strcpy_from_child(tcp, addr + sizeof(addrbuf.sau.sun_family));
      if (original_path) {
        //printf("original_path='%s'\n", original_path);

        char* redirected_path =
          redirect_filename_into_cderoot(original_path, tcp->current_dir, tcp);

        // could be null if path is being ignored by cde.options
        if (redirected_path) {
          //printf("redirected_path: '%s'\n", redirected_path);

          unsigned long new_pathlen = strlen(redirected_path);

          // alter the socket address field to point to redirected path
          memcpy_to_child(tcp->pid, (char*)(addr + sizeof(addrbuf.sau.sun_family)),
                          redirected_path, new_pathlen + 1);

          free(redirected_path);


          // remember the 2 extra bytes for the sun_family field!
          unsigned long new_totallen = new_pathlen + sizeof(addrbuf.sau.sun_family);

          struct user_regs_struct cur_regs;
          EXITIF(ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)&cur_regs) < 0);

#if defined (I386)
          // on i386, things are tricky tricky!
          // the kernel uses socketcall() as a common entry
          // point for all socket-related system calls
          // http://www.kernel.org/doc/man-pages/online/pages/man2/socketcall.2.html
          //
          // the ecx register contains a pointer to an array of 3 pointers
          // (of size 'unsigned long'), which represents the 3 arguments
          // to the bind/connect syscall.  they are:
          //   arg[0] - socket number
          //   arg[1] - pointer to socket address structure
          //   arg[2] - length of socket address structure

          // we need to alter the length field to new_totallen,
          // which is VERY IMPORTANT or else the path that the
          // kernel sees will be truncated!!!

          // we want to override arg[2], which is located at:
          //   cur_regs.ecx + 2*sizeof(unsigned long)
          memcpy_to_child(tcp->pid, (char*)(cur_regs.ecx + 2*sizeof(unsigned long)),
                          (char*)(&new_totallen), sizeof(unsigned long));
#elif defined(X86_64)
          // on x86-64, things are much simpler.  the length field is
          // stored in %rdx (the third argument), so simply override
          // that register with new_totallen
          cur_regs.rdx = (long)new_totallen;
          ptrace(PTRACE_SETREGS, tcp->pid, NULL, (long)&cur_regs);
#else
          #error "Unknown architecture (not I386 or X86_64)"
#endif
        }

        free(original_path);
      }
    }
  }
  else {
    if (CDE_block_net_access) {
      // blank out the sockaddr argument if you want to block network access
      //
      // I think that blocking 'bind' prevents setting up sockets to accept
      // incoming connections, and blocking 'connect' prevents outgoing
      // connections.
      struct sockaddr s;
      memset(&s, 0, sizeof(s));
      memcpy_to_child(tcp->pid, (char*)addr, (char*)&s, sizeof(s));
    }
  }
}
