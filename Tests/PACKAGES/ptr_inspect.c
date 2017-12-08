/*
  ptr_inspect.c

  Demonstration code; shows how to trace the system calls in a child
  process with ptrace.  Only works on 64-bit x86 Linux for now, I'm
  afraid.  (Even worse, it's only tested on Linux 2.6....) 
 
  The callname() function looks clunky and machine-generated because it
  *is* clunky and machine-generated.

  I got inspiration and a starting point from this old LJ article:
    http://www.linuxjournal.com/article/6100 

  This code is in the public domain.  Share and enjoy.

  Will Benton
  Madison, 2008
*/

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <syscall.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <callname.h>
#include <sys/reg.h>
int main(int argc, char* argv[]) {   
  pid_t child;

  if (argc == 1) {
    exit(0);
  }

  char* chargs[argc];
  int i = 0;

  while (i < argc - 1) {
    chargs[i] = argv[i+1];
    i++;
  }
  chargs[i] = NULL;

  child = fork();
  if(child == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execvp(chargs[0], chargs);
  } else {
    int status;

    while(waitpid(child, &status, 0) && ! WIFEXITED(status)) {
      struct user_regs_struct regs; 
      ptrace(PTRACE_GETREGS, child, NULL, &regs);
      fprintf(stderr, "system call %s from pid %d\n", callname(REG(regs)), child);
          long params[4];
          params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * RAX,
                                   NULL);
          params[1] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * RBX,
                                   NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * RCX,
                                   NULL);
                params[3] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * RDX,
                                   NULL);
      if (callname(REG(regs)) == "open")
      {
          printf("OPEN DETECTED, Open called with %ld, %ld, %ld\n", params[0], params[1], params[2]);
                           ptrace(PTRACE_GETREGS, child,
                        NULL, &regs);
                 printf("Open called with "
                        "%ld, %ld, %ld\n",
                        regs.rbx, regs.rcx,
                        regs.rdx);
      }
      if (callname(REG(regs)) == "close")
      {
          printf("CLOSE DETECTED, Close called with %ld, %ld, %ld\n", params[0], params[1], params[2]);
                 printf("close called with "
                        "%ld, %ld, %ld\n",
                        regs.rbx, regs.rcx,
                        regs.rdx);
      }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }
  }
  exit(0);
}
