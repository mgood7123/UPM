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
        printf("Usage: %s <prog> <arg1> ... <argN>\n", argv[0]);
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
//         printf("*******************START*****************\n");
        if (wait_for_syscall(child) != 0) break;
//         printf("*******************MIDDLE*****************\n");
        /* Wait for syscall exit */
        if (wait_for_syscall(child) != 0) break;
//         printf("*******************END*****************\n");
    }
}

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
// rewrite_syscall(write, child, 2, "TEST"); // for open(), need to modify for write() syscall

char * rewrite_syscall(const char * syscall, int pid) {
    char * set_reg(int arg) {
        if ( arg == 1) {
            return RDI;
        } else if ( arg == 2) {
            return RSI;
        } else if ( arg == 3) {
            return RDX;
        } else if ( arg == 4) {
            return R10;
        } else if ( arg == 5) {
            return R8;
        } else if ( arg == 6) {
            return R9;
        } else {
            printf("\nARG INVALID: %i\n", arg);
            return NULL;
        }
    }
//             regs.rdi - Stores the first argument
//
//             regs.rsi - Stores the second argument
//
//             regs.rdx - Stores the third argument
//
//             regs.r10 - Stores the fourth argument
//
//             regs.r8 - Stores the fifth argument
//
//             regs.r9 - Stores the sixth argument
    // 0 on _syscall will allow all syscalls to be read and modified
    // 1 on _syscall will prevent all syscalls from being read or modified globally

    // 0 will allow the specified syscall to be modified
    // 1 will prevent modification of the syscall
    const char * _syscall = "0";

    const char * _write = "1";
    const char * _read = "0";
    const char * _open = "0";
    const char * _openat = "0";
    const char * _stat = "0";
    const char * _lstat = "0";
    const char * _exec = "0";
    const char * _sigsuspend = "1";
    char syscall_modification_not_supported(const char * syscallnum)
    {
//         printf ("SYSCALL %s() MODIFICATION IS NOT YET SUPPORTED from pid %d\n", syscallnum, pid);
    }
    char syscall_not_supported(const char * syscallnum)
    {
//         printf ("SYSCALL %s() IS NOT YET SUPPORTED from pid %d\n", syscallnum, pid);
    }
    void read_file(pid_t child, char *file, int arg)
    {
        // READS A "const char"/"char" TYPE VALUE
        char *child_addr;
        int i;
        char regcode = set_reg(arg);
        child_addr = (char *) ptrace(PTRACE_PEEKUSER, child, sizeof(long)*regcode, 0);

        do {
            long val;
            char *p;

            val = ptrace(PTRACE_PEEKTEXT, child, child_addr, NULL);
            if (val == -1) {
                printf("PTRACE_PEEKTEXT error: %s\n", strerror(errno));
                break;
            }
            child_addr += sizeof (long);

            p = (char *) &val;
            for (i = 0; i < sizeof (long); ++i, ++file) {
                *file = *p++;
//                 printf(file);
//                 printf("\n");
                if (*file == '\0') break;
            }
        } while (i == sizeof (long));
    }
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    char regcode;
    int arg;
    int value;
    const char * replacement;
    if (_syscall == "1") {
        syscall_not_supported(syscall);
    } else if (_syscall == "0") {
        if (syscall == "write") {
            arg = 2;
            regcode = set_reg(arg);
            replacement = "IDEK\n";
            value = strlen(replacement);
            char message[1000];
            char* temp_char2 = message;
            int j = 0;
            long temp_long;
            // printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
            while( j < regs.rdx ) //regs.rdx stores the size of the input buffer
            {
                temp_long = ptrace(PTRACE_PEEKDATA, pid, regs.rsi + (j*8) , NULL);
                memcpy(temp_char2, &temp_long, 8);
                temp_char2 += sizeof(long);
                ++j;
            }
            message[regs.rdx] = '\0';
            printf("\nORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
            printf("TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, str_replace(message, "\n", "\\n"), regs.rdx, pid);
            if (_write == "0") {
                printf("ATTEMPTING TO REWRITE SYSTEM CALL %s argument %i (register: rdi)\n", syscall, arg);

                char *stack_addr, *file_addr;
                stack_addr = (char *) ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RSP, 0);
                /* Move further of red zone and make sure we have space for the file name */
                stack_addr -= 128 + PATH_MAX;
                file_addr = stack_addr;
                /* Write new file in lower part of the stack */

                if ( arg == 2 ) {
    //                 SETS A CHAR "" TYPE (eg ... "TEST" ...
                    do {
                        int i;
                        char val[sizeof (long)];
                        for (i = 0; i < sizeof (long); ++i, ++replacement) {
                            val[i] = *replacement;
                            if (*replacement == '\0') break;
                        }
                        ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                        stack_addr += sizeof (long);
                    } while (*replacement);
                    do {
                        int i;
                        char val[sizeof (long)];
                        for (i = 0; i < sizeof (long); ++i, ++replacement) {
                            val[i] = *replacement;
                            if (*replacement == '\0') break;
                        }
                        ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                        stack_addr += sizeof (long);
                    } while (*replacement);
                    /* Change argument to open */
                    if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*regcode, file_addr) != 0) {
                        printf("MODIFICATION FAILED\n");
                    }
                    if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*RDX, value) != 0) {
                        printf("MODIFICATION FAILED\n");
                    }
                }
                struct user_regs_struct regsb;
                ptrace(PTRACE_GETREGS, pid, NULL, &regsb);
                char messageb[1000];
                char* temp_char2b = messageb;
                int jb = 0;
                long temp_longb;

                while( jb < regs.rdx ) //regs.rdx stores the size of the input buffer
                {
                    temp_longb = ptrace(PTRACE_PEEKDATA, pid, regsb.rsi + (jb*8) , NULL);
                    memcpy(temp_char2b, &temp_longb, 8);
                    temp_char2b += sizeof(long);
                    ++jb;
                }
                messageb[regs.rdx] = '\0';
    //             printf("NEW system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
                printf("ORIGINAL TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, str_replace(message, "\n", "\\n"), regs.rdx, pid);
                printf("NEW TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regsb.orig_rax, callname(REG(regs)), regsb.rdi, str_replace(messageb, "\n", "\\n"), regsb.rdx, pid);
            } else {
                syscall_modification_not_supported(syscall);
            }
        } else if (syscall == "open") {
            arg = 1;
            regcode = set_reg(arg);
    //         int value = strlen(replacement);
    //         write(fd,string,size);
    //         write(1, "test", sizeof("test"));
            char *messageb[PATH_MAX];
            char *message = messageb;
            char* temp_char2 = message;
            int j = 0;
            long temp_long;
            // printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
            read_file(pid, message, arg);
    //         printf("ORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
            // printf("TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), message, regs.rsi, regs.rdx, pid);
            if (_open == "0") {
                char *replacement = NULL;
                asprintf (&replacement, ".%s", messageb);
                if(!access(replacement, F_OK)) {
                    const char * replacementB = replacement;
                    printf("%s exists\n", replacement);
                    printf("ATTEMPTING TO REWRITE SYSTEM CALL %s argument %i\n", syscall, arg);
                    char *stack_addr, *file_addr;
                    stack_addr = (char *) ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RSP, 0);
                    /* Move further of red zone and make sure we have space for the file name */
                    stack_addr -= 128 + PATH_MAX;
                    file_addr = stack_addr;
                    /* Write new file in lower part of the stack */
                    if ( arg == 1 ) {
        //                 SETS A CHAR "" TYPE (eg ... "TEST" ...
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        /* Change argument to open */
                        if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*regcode, file_addr) != 0) {
                            printf("MODIFICATION FAILED\n");
                        }
                    }
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                    char * messageb[PATH_MAX];
                    read_file(pid, messageb, arg);
    //                 printf("NEW system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
                    printf("ORIGINAL TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), message, regs.rsi, regs.rdx, pid);
                    printf("NEW TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), messageb, regs.rsi, regs.rdx, pid);
                    printf("does replacementB (%s) match messageb (%s)?\n", replacementB, messageb);
                    if (strcmp(messageb, replacementB) == 0) {
                        printf("YES\n");
                    } else {
                        printf("replacementB (%s) does NOT match messageb (%s)\n", replacementB, messageb);
                    }
                } else {
                    printf("%s does not exist, aborting call modification\n", replacement);
                }
            } else {
                syscall_modification_not_supported(syscall);
            }
        } else if (syscall == "openat") {
            arg = 2;
            regcode = set_reg(arg);
    //         int value = strlen(replacement);
    //         write(fd,string,size);
    //         write(1, "test", sizeof("test"));
            char *messageb[PATH_MAX];
            char *message = messageb;
            char* temp_char2 = message;
            int j = 0;
            long temp_long;
            printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
            read_file(pid, message, arg);
    //         printf("\nORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
            printf("TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, str_replace(message, "\n", "\\n"), regs.rdx, pid);
            if (_openat == "0") {
                char *replacement = NULL;
                char current_pwd[PATH_MAX];
                getcwd(current_pwd, sizeof(current_pwd));
                asprintf (&replacement, "%s%s", current_pwd, messageb);
                if(!access(replacement, F_OK)) {
                    const char * replacementB = replacement;
                    printf("%s exists\n", replacement);
                    printf("ATTEMPTING TO REWRITE SYSTEM CALL %s argument %i\n", syscall, arg);
                    char *stack_addr, *file_addr;
                    stack_addr = (char *) ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RSP, 0);
                    /* Move further of red zone and make sure we have space for the file name */
                    stack_addr -= 128 + PATH_MAX;
                    file_addr = stack_addr;
                    /* Write new file in lower part of the stack */
    //                 SETS A CHAR "" TYPE (eg ... "TEST" ...
                    do {
                        int i;
                        char val[sizeof (long)]; // defaults to 8 on x86_64, 4 on x86
                        for (i = 0; i < sizeof (long); ++i, ++replacement) {
                            val[i] = *replacement;
                            if (*replacement == '\0') break;
                        }
                        ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                        stack_addr += sizeof (long);
                    } while (*replacement);
                    do {
                        int i;
                        char val[sizeof (long)]; // defaults to 8 on x86_64, 4 on x86
                        for (i = 0; i < sizeof (long); ++i, ++replacement) {
                            val[i] = *replacement;
                            if (*replacement == '\0') break;
                        }
                        ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                        stack_addr += sizeof (long);
                    } while (*replacement);
                    /* Change argument to open */
                    if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*regcode, file_addr) != 0) {
                        printf("MODIFICATION FAILED\n");
                    }
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                    char * messageb[PATH_MAX];
                    read_file(pid, messageb, arg);
    //                 printf("NEW system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
                    printf("ORIGINAL TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, message, regs.rdx, pid);
                    printf("NEW TRANSLATED system call (start) (call number: %ld) %s(%ld, \"%s\", %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, messageb, regs.rdx, pid);
                    printf("does replacementB (%s) match messageb (%s)?\n", replacementB, messageb);
                    if (strcmp(messageb, replacementB) == 0) {
                        printf("YES\n");
                    } else {
                        printf("replacementB (%s) does NOT match messageb (%s)\n", replacementB, messageb);
                    }
                } else {
                    printf("%s does not exist, aborting call modification\n", replacement);
                }
            } else {
                syscall_modification_not_supported(syscall);
            }
        } else if (syscall == "stat" || syscall == "lstat") {
            arg = 1;
            regcode = set_reg(arg);
    //         int value = strlen(replacement);
    //         write(fd,string,size);
    //         write(1, "test", sizeof("test"));
            char *messageb[PATH_MAX];
            char *message = messageb;
            char* temp_char2 = message;
            int j = 0;
            long temp_long;
            // printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
            read_file(pid, message, arg);
    //         printf("ORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
            // printf("TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), message, regs.rsi, regs.rdx, pid);
            if (_stat == "SUPPORTED" || _lstat == "0") {
                char *replacement = NULL;
                asprintf (&replacement, ".%s", messageb);
                if(!access(replacement, F_OK)) {
                    const char * replacementB = replacement;
                    printf("%s exists\n", replacement);
                    printf("ATTEMPTING TO REWRITE SYSTEM CALL %s argument %i\n", syscall, arg);
                    char *stack_addr, *file_addr;
                    stack_addr = (char *) ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RSP, 0);
                    /* Move further of red zone and make sure we have space for the file name */
                    stack_addr -= 128 + PATH_MAX;
                    file_addr = stack_addr;
                    /* Write new file in lower part of the stack */
                    if ( arg == 1 ) {
        //                 SETS A CHAR "" TYPE (eg ... "TEST" ...
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        /* Change argument to open */
                        if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*regcode, file_addr) != 0) {
                            printf("MODIFICATION FAILED\n");
                        }
                    }
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                    char * messageb[PATH_MAX];
                    read_file(pid, messageb, arg);
    //                 printf("NEW system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
                    printf("ORIGINAL TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), messageb, regs.rsi, regs.rdx, pid);
                    printf("NEW TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), messageb, regs.rsi, regs.rdx, pid);
                    printf("does replacementB (%s) match messageb (%s)?\n", replacementB, messageb);
                    if (strcmp(messageb, replacementB) == 0) {
                        printf("YES\n");
                    } else {
                        printf("replacementB (%s) does NOT match messageb (%s)\n", replacementB, messageb);
                    }
                } else {
                    printf("%s does not exist, aborting call modification\n", replacement);
                }
            } else {
                syscall_modification_not_supported(syscall);
            }
        } else if (syscall == "rt_sigsuspend") {
            printf("ORIGINAL system call (start) (call number: %ld) %s(%ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, pid);
            printf("ATTEMPTING TO SEND SIGNAL\n");
            if (ptrace(PTRACE_CONT, pid, 0, SIGCONT) == -1) {
                printf("SIGNAL FAILED\n");
            }
        } else if (syscall == "execl" || syscall == "execlp" || syscall == "execle" || syscall == "execv" || syscall == "execve" || syscall == "execvp" || syscall == "execvpe" ) {
            arg = 1;
            regcode = set_reg(arg);
    //         int value = strlen(replacement);
    //         write(fd,string,size);
    //         write(1, "test", sizeof("test"));
            char *messageb[PATH_MAX];
            char *message = messageb;
            char* temp_char2 = message;
            int j = 0;
            long temp_long;
            printf("ATTEMPTING TO READ SYSTEM CALL %s argument %i (REGISTER: rdi)\n", syscall, arg);
            read_file(pid, message, arg);
    //         printf("ORIGINAL system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
            printf("TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), message, regs.rsi, regs.rdx, pid);
            if (_exec == "0") {
                char *replacement = NULL;
                asprintf (&replacement, ".%s", messageb);
                if(!access(replacement, F_OK)) {
                    const char * replacementB = replacement;
                    printf("%s exists\n", replacement);
                    printf("ATTEMPTING TO REWRITE SYSTEM CALL %s argument %i\n", syscall, arg);
                    char *stack_addr, *file_addr;
                    stack_addr = (char *) ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*RSP, 0);
                    /* Move further of red zone and make sure we have space for the file name */
                    stack_addr -= 128 + PATH_MAX;
                    file_addr = stack_addr;
                    /* Write new file in lower part of the stack */
                    if ( arg == 1 ) {
        //                 SETS A CHAR "" TYPE (eg ... "TEST" ...
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        do {
                            int i;
                            char val[sizeof (long)];
                            for (i = 0; i < sizeof (long); ++i, ++replacement) {
                                val[i] = *replacement;
                                if (*replacement == '\0') break;
                            }
                            ptrace(PTRACE_POKETEXT, pid, stack_addr, *(long *) val);
                            stack_addr += sizeof (long);
                        } while (*replacement);
                        /* Change argument to open */
                        if (ptrace(PTRACE_POKEUSER, pid, sizeof(long)*regcode, file_addr) != 0) {
                            printf("MODIFICATION FAILED\n");
                        }
                    }
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                    char * messageb[PATH_MAX];
                    read_file(pid, messageb, arg);
    //                 printf("NEW system call (start) (call number: %ld) %s(%ld, %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, regs.rsi, regs.rdx, pid);
                    printf("ORIGINAL TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), messageb, regs.rsi, regs.rdx, pid);
                    printf("NEW TRANSLATED system call (start) (call number: %ld) %s(\"%s\", %ld, %ld) from pid %d\n", regs.orig_rax, callname(REG(regs)), messageb, regs.rsi, regs.rdx, pid);
                    printf("does replacementB (%s) match messageb (%s)?\n", replacementB, messageb);
                    if (strcmp(messageb, replacementB) == 0) {
                        printf("YES\n");
                    } else {
                        printf("replacementB (%s) does NOT match messageb (%s)\n", replacementB, messageb);
                    }
                } else {
                    printf("%s does not exist, aborting call modification\n", replacement);
    //                 perror("access failed 1");
    //                 exit(0);
                }
            } else {
                syscall_modification_not_supported(syscall);
            }
        } else {
//             printf("\nsystem call (start) (call number: %ld) %s (with args: ARG1: rdi: %ld, size of rdi:%ld, ARG2: rsi:%ld, size of rsi:%ld, ARG3: rdx:%ld, size of rdx:%ld, ARG4: r10:%ld, size of r10:%ld, ARG5: r8:%ld, size of r8:%ld, ARG6: r9:%ld, size of r9:%ld, ) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rdi, sizeof(regs.rdi), regs.rsi, sizeof(regs.rsi), regs.rdx, sizeof(regs.rdx), regs.r10, sizeof(regs.r10), regs.r8, sizeof(regs.r8), regs.r9, sizeof(regs.r9), pid);
//             printf("\nsystem call (start) (call number: %ld) %s from pid %d\n", regs.orig_rax, callname(REG(regs)), pid);
            syscall_not_supported(syscall);
        }
    }
}
static int wait_for_syscall(pid_t child)
{
    int status;
    int toggle = 0;
    int signal;

    while (1) {
        signal = 0;
        if (WIFSTOPPED(status)) {
            if (WSTOPSIG(status) != SIGTRAP|0x80) {
                signal = WSTOPSIG(status);
                printf("recieved signal %i\n", signal);
            }
        }
        ptrace(PTRACE_SYSCALL, child, 0, signal);
        if (waitpid(-1, &status, __WALL) == -1);
        {
//             printf("syscall waitpid failed with -1\n");
        }
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1) { printf("\n\n[ERROR] 2nd ptrace returned -1 from %i\n\n\n", child); }
        int err_no = regs.rax;
        if (toggle == 0) {
            if (err_no == -38) {
                toggle = 1;
                rewrite_syscall(callname(REG(regs)), child);
            }
        }
        else {
            toggle = 0;
            // clone() returns its new pid here as an %i
            if (callname(REG(regs)) == "clone") {
                int pid_child = regs.rax;
                if (pid_child > 0 ) {
                fprintf("clone detected, new pid of cloned process is %i", regs.rax);
                printf("\npid_child: %i", pid_child);
                printf("\nattempting to attach to %i\n", pid_child);
                ptrace(PTRACE_ATTACH, pid_child, 0, 0);
                ptrace(PTRACE_SYSCALL, pid_child, 0, 0);
                process_signals(pid_child); // WARNING this causes the entire application to become desyncronized with the real time system calls (including the standard system calls) which effectively disables the ability to modify system calls
                } else {
                    printf("syscall clone returned an invalid pid (possible error code): (as long double) %ld, (as int) %i", pid_child);
                }
//                 http://codepad.org/LGXGkodS gcc ./redirect.c -o redir && (download strace, cd strace) ./build_static_example.sh ; ./configure && make && cd tests && make check TESTS='rt_sigsuspend.gen' && strace redir ./rt_sigsuspend

// https://bpaste.net/raw/8db405dd4c96

// ‎[15:56] ‎<‎eSyr-ng_‎>‎ No, really, there are about two well-known users of ptrace(2) — strace and gdb, and their ptrace-related code is manifestation of all the ptrace(2) quirks presented in Linux kernel over time, implemented in code. I'd really suggest…
// ‎[15:56] ‎<‎eSyr-ng_‎>‎ … inspecting this code before trying to implement some non-trivial ptrace stuff yourself.
// ‎[15:57] ‎<‎eSyr-ng_‎>‎ well, if tracee was PTRACE_SEIZE'd, this should probably work.
// ‎[15:57] ‎<‎laggy_wifi‎>‎ now i get it only 3 times 1st ptrace returned -1 from 19344
// ‎[15:58] ‎<‎laggy_wifi‎>‎ does PTRACE_INTERRUPT and PTRACE_CONT return -1 on error
// ‎[15:59] ‎<‎laggy_wifi‎>‎ like failing to stop and failing to continue
// ‎[16:00] ‎<‎laggy_wifi‎>‎ wait i should not need to skip the wait4 syscall
// ‎[16:05] ‎<‎laggy_wifi‎>‎ ok so rt_sigsuspend is not exiting (otherwise i would get a -1 as the process does not exist)
// ‎[16:05] ‎<‎laggy_wifi‎>‎ ptrace(PTRACE_PEEKUSER, 19743, 8*ORIG_RAX, [0x82]) = 0
// ‎[16:05] ‎<‎laggy_wifi‎>‎ ptrace(PTRACE_SYSCALL, 19743, NULL, SIG_0) = 0
// ‎[16:05] ‎<‎laggy_wifi‎>‎ wait4(-1, 
// ‎[16:06] ‎<‎laggy_wifi‎>‎ "if (waitpid(-1, &status, __WALL) == -1);"
// ‎[16:07] ‎<‎laggy_wifi‎>‎ so fer every wait4 is either "wait4(-1, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGTRAP | 0x80}], __WALL, NULL) = 19827" or "wait4(-1, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGTRAP}], __WALL, NULL) = 19827"
// ‎[16:07] ‎<‎laggy_wifi‎>‎ far*
// ‎[16:08] ‎<‎laggy_wifi‎>‎ (or "wait4(-1, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGUSR1}], __WALL, NULL) = 19827")
// ‎[16:09] ‎<‎laggy_wifi‎>‎ https://bpaste.net/raw/feeedbf93109


// could rt_sigsuspend interfer with a &status in any case? eg "wait4(-1, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGTRAP | 0x80}], __WALL, NULL) = 19827" or "wait4(-1, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGTRAP}], __WALL, NULL) = 19827" as strace just shows "wait4(-1," and nothing after as if it has hanged, (in wich it does hang even without strace)

            } else {
//                 printf("\nsystem call (end) (call number: %ld) %s (with return: %i) from pid %d\n", regs.orig_rax, callname(REG(regs)), regs.rax, child);
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
