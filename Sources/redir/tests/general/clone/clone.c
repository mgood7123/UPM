#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    struct utsname uts;
    pid_t pid = getpid();

    /* Change hostname in UTS namespace of child */

//     if (sethostname(arg, strlen(arg)) == -1)
//         errExit("sethostname");

    /* Retrieve and display hostname */

    if (uname(&uts) == -1)
        errExit("uname");
    printf("uts.nodename in child:  %s from current pid: %d\n", uts.nodename, pid);
    printf("child\n");

    /* Keep the namespace open for a while, by sleeping.
        This allows some experimentation--for example, another
        process might join the namespace. */

    sleep(5);

    return 0;           /* Child terminates now */

}

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

int
main(int argc, char *argv[])
{
    char *stack;                    /* Start of stack buffer */
    char *stackTop;                 /* End of stack buffer */
    pid_t pid;
    pid_t CurrentPid = getpid();
    struct utsname uts;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Allocate stack for child */

    stack = malloc(STACK_SIZE);
    if (stack == NULL)
        errExit("malloc");
    stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

    /* Create child that has its own UTS namespace;
        child commences execution in childFunc() */
//     printf("THIS PRINTF SHOULD BE CACHED AND REPLACED WITH IDEK BEFORE THE CLONE() IS CALLED from current pid: %d\n", CurrentPid);
    printf("parent1\n");
    pid = clone(childFunc, stackTop, SIGCHLD, argv[1]);
//     printf("THIS PRINTF SHOULD BE CACHED AND REPLACED WITH IDEK AFTER THE CLONE() IS CALLED from current pid: %d\n", CurrentPid);
    printf("parent2\n");
    if (pid == -1)
        errExit("clone");
//     printf("clone() returned %ld\n", (long) pid);

    /* Parent falls through to here */

    sleep(1);           /* Give child time to change its hostname */

    /* Display hostname in parent's UTS namespace. This will be
        different from hostname in child's UTS namespace. */

    if (uname(&uts) == -1)
        errExit("uname");
    printf("uts.nodename in parent: %s from current pid: %d\n", uts.nodename, CurrentPid);
    printf("parent3\n");

    if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
        errExit("waitpid");
//     printf("child has terminated from current pid: %d\n", CurrentPid);
    printf("parent4\n");

//     printf("THIS PRINTF SHOULD ALSO BE CACHED AND REPLACED WITH IDEK AFTER THE CLONE() IS CALLED from current pid: %d\n", CurrentPid);
    printf("parent5\n");
    exit(EXIT_SUCCESS);
}
