/****************
 Example program to demonstrate use of FIFOs (named pipes)
 ****************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void errexit(char *errMsg){
  printf("\n About to exit: %s", errMsg);
  fflush(stdout);
  exit(1);
}

/*******
 A FIFO is created by the mknod system call.
 int mknod(char *pathname, int mode, int dev);
 The pathname is a normal UNIX pathname and is the name of the FIFO.
 The mode specifies permissions for the FIFO (owner, group and world) and is logically or'ed with 
 the S_IFIFO flag.
 The dev value is ignored for a FIFO.

 Once a FIFO is created, it may be opened for reading or writing using standard calls: open, fopen etc.

 An open call that opens the FIFO read-only blocks until some other process opens the FIFO for writing.
 An open call that opens the FIFO write-only blocks until some other process opens the FIFO for reading.
 A read call on a FIFO blocks until there is data in the FIFO or until no processes have it open for writing.
 A write call on a FIFO blocks until there is space available.
 If a process writes to a FIFO but there are no processes in existence that have it open for reading, the 
 SIGPIPE signal is generated and the write returns zero with errno set to EPIPE. If the process has not
 called signal to handle SIGPIPE, the default action is to terminate the process.


 In the program below, the parent creates a FIFO, forks off a child and opens the FIFO for reading. The child
 opens the FIFO for writing and writes an integer onto it. The parent reads the integer and prints it out.
  *********/
 
int main()
{
    int ret;
    pid_t pid;
    int value;
    char fifoName[]="/tmp/testfifo";
    char errMsg[1000];
    FILE *cfp;
    FILE *pfp;

    ret = mknod(fifoName, S_IFIFO | 0600, 0); 
    /* 0600 gives read, write permissions to user and none to group and world */
    if(ret < 0){
      sprintf(errMsg,"Unable to create fifo: %s",fifoName);
      errexit(errMsg);
    }

    pid=fork();
    if(pid == 0){
	/* child -- open the named pipe and write an integer to it */

      cfp = fopen(fifoName,"w");
      if(cfp == NULL) 
	errexit("Unable to open fifo for writing");
      ret=fprintf(cfp,"%d",1000);
      fflush(cfp);
      exit(0);
    } 

    else{
      /* parent - open the named pipe and read an integer from it */
      pfp = fopen(fifoName,"r");
      if(pfp == NULL) 
	errexit("Unable to open fifo for reading");
      ret=fscanf(pfp,"%d",&value);
      if(ret < 0) 
	errexit("Error reading from named pipe");
      fclose(pfp);
      printf("This is the parent. Received value %d from child on fifo \n", value);
      unlink(fifoName); /* Delete the created fifo */
      exit(0);
    }
}

