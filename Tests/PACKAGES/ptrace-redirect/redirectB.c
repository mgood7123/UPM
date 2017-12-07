#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <signal.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <callname.h>

static void process_signals(pid_t child);
static int wait_for_syscall(pid_t child);
static void read_file(pid_t child, char *file);
static void redirect_file(pid_t child, const char *file);

int main(int argc, char **argv)
{
    pid_t pid;
    int status;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prog> <arg1> ... <argN>\n", argv[0]);
        return 1;
    }
    if ((pid = fork()) == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        kill(getpid(), SIGSTOP);
        return execvp(argv[1], argv + 1);
    } else {
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
    int status;
    int toggle = 0;

    while (1) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        if (waitpid(-1, &status, 0) == -1);
        {
//             printf("syscall waitpid failed with -1\n");
        }
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        if(toggle == 0) {
            toggle = 1;
if (callname(REG(regs)) == "write") {
                char message[1000];
                char* temp_char2 = message;
                int j = 0;
                long temp_long;
                // printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
                while( j < regs.rdx ) //regs.rdx stores the size of the input buffer
                {
                    temp_long = ptrace(PTRACE_PEEKDATA, child, regs.rsi + (j*8) , NULL);
                    memcpy(temp_char2, &temp_long, 8);
                    temp_char2 += sizeof(long);
                    ++j;
                }
                message[regs.rdx] = '\0';
                char *str_replace(char *orig, char *rep, char *with) {
                char *result; // the return string
                char *ins;    // the next insert point
                char *tmp;    // varies
                int len_rep;  // length of rep (the string to remove)
                int len_with; // length of with (the string to replace rep with)
                int len_front; // distance between rep and end of last rep
                int count;    // number of replacements

                // sanity checks and initialization
                if (!orig || !rep)
                    return NULL;
                len_rep = strlen(rep);
                if (len_rep == 0)
                    return NULL; // empty rep causes infinite loop during count
                if (!with)
                    with = "";
                len_with = strlen(with);

                // count the number of replacements needed
                ins = orig;
                for (count = 0; tmp = strstr(ins, rep); ++count) {
                    ins = tmp + len_rep;
                }

                tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

                if (!result)
                    return NULL;

                // first time through the loop, all the variable are set correctly
                // from here on,
                //    tmp points to the end of the result string
                //    ins points to the next occurrence of rep in orig
                //    orig points to the remainder of orig after "end of rep"
                while (count--) {
                    ins = strstr(orig, rep);
                    len_front = ins - orig;
                    tmp = strncpy(tmp, orig, len_front) + len_front;
                    tmp = strcpy(tmp, with) + len_with;
                    orig += len_front + len_rep; // move to next "end of rep"
                }
                strcpy(tmp, orig);
                return result;
            }
                fprintf(stderr, "\nORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, child);
                fprintf(stderr, "TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, str_replace(message, "\n", "\\n"), regs.rdx, child);
}
//             rewrite_syscall(callname(REG(regs)), child);
        }
        else {
            toggle = 0;
            if (callname(REG(regs)) == "clone") {
                fprintf(stderr, "clone detected, new pid of cloned process is %i", regs.rax);
                int pid_child = regs.rax;
                fprintf(stderr, "\npid_child: %i", pid_child);
                fprintf(stderr, "\nattempting to attach to %i\n", pid_child);
                ptrace(PTRACE_ATTACH, pid_child, 0, 0);
                ptrace(PTRACE_SYSCALL, pid_child, 0, 0);
                process_signals(pid_child); // works but it seems to hang and not let the parent output anything after the attach
            } else if (callname(REG(regs)) == "write") {
                char message[1000];
                char* temp_char2 = message;
                int j = 0;
                long temp_long;
                // printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
                while( j < regs.rdx ) //regs.rdx stores the size of the input buffer
                {
                    temp_long = ptrace(PTRACE_PEEKDATA, child, regs.rsi + (j*8) , NULL);
                    memcpy(temp_char2, &temp_long, 8);
                    temp_char2 += sizeof(long);
                    ++j;
                }
                message[regs.rdx] = '\0';
                char *str_replace(char *orig, char *rep, char *with) {
                char *result; // the return string
                char *ins;    // the next insert point
                char *tmp;    // varies
                int len_rep;  // length of rep (the string to remove)
                int len_with; // length of with (the string to replace rep with)
                int len_front; // distance between rep and end of last rep
                int count;    // number of replacements

                // sanity checks and initialization
                if (!orig || !rep)
                    return NULL;
                len_rep = strlen(rep);
                if (len_rep == 0)
                    return NULL; // empty rep causes infinite loop during count
                if (!with)
                    with = "";
                len_with = strlen(with);

                // count the number of replacements needed
                ins = orig;
                for (count = 0; tmp = strstr(ins, rep); ++count) {
                    ins = tmp + len_rep;
                }

                tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

                if (!result)
                    return NULL;

                // first time through the loop, all the variable are set correctly
                // from here on,
                //    tmp points to the end of the result string
                //    ins points to the next occurrence of rep in orig
                //    orig points to the remainder of orig after "end of rep"
                while (count--) {
                    ins = strstr(orig, rep);
                    len_front = ins - orig;
                    tmp = strncpy(tmp, orig, len_front) + len_front;
                    tmp = strcpy(tmp, with) + len_with;
                    orig += len_front + len_rep; // move to next "end of rep"
                }
                strcpy(tmp, orig);
                return result;
            }
                fprintf(stderr, "\nORIGINAL system call (end) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, child);
                fprintf(stderr, "TRANSLATED system call (end) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, str_replace(message, "\n", "\\n"), regs.rdx, child);
            } else {
//                 fprintf(stderr, "\nsystem call (end) (call number: %ld) %s (with return: %i) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rax, child);
                fprintf(stderr, "\nsystem call (end) (call number: %ld) %s (with return: %i) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rax, child);
            }
        }
        if (WEXITSTATUS(status))
        {
//             printf("exited with status %ld\n", status);
        }
        /* Is it the open syscall (sycall number 2 in x86_64)? */
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80 &&
            ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX, 0) == 2)
            return 0;
        if (WIFEXITED(status))
            return 1;
    }
}
