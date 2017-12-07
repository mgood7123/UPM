#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <fcntl.h>
#include <linux/limits.h>

static void process_signals(pid_t child);
static int wait_for_syscall(pid_t child);

int main(int argc, char **argv)
{
    pid_t pid;
    int status;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prog> <arg1> ... <argN>\n", argv[0]);
        return 1;
    }
    if ((pid = fork()) == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        kill(getpid(), SIGSTOP);
        return execvp(argv[1], argv + 1);
    } else {
        printf ("tracing %i\n", pid);
        waitpid(pid, &status, 0);
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACECLONE);
        process_signals(pid);
        return 0;
    }
}

static void process_signals(pid_t child)
{
    while(1) {
        /* Wait for syscall start */
        if (wait_for_syscall(child) != 0) break;
        /* Wait for syscall exit */
        if (wait_for_syscall(child) != 0) break;
    }
}

static int wait_for_syscall(pid_t child)
{
    while (1) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
    }
}
