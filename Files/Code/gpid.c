	
// A process can set the process group ID of only itself or any of its children. Furthermore, it can't change the process group ID of one of its children after that child has called one of the exec functions. --APUE
// 
// In my opinion,
// 
// 1.a grandparent can't use setgpid() with its gradechild, you can check this easily.That's to say, the code in pid 0 below won't work:
// 
// setpgid(pid3, pid3); 
// setpgid(pid4, pid3);
// setpgid(pid5, pid3);
// 2.you can only use setgpid() to change one's and itselves chilld pgid,you can't write down setpgid(pid5, pid3) in pid 3, because pid 3 and pid 5 aren't parent and child.
// 
// So, you'd better use setgpid(someone's pid, pgid) in itself.
// 
// But how can one process know other processes' pid? A method is shared memory.
// 
// Here is one rough but a litte complex implement I just wrote, which don't consider process synchronization.It works as you expected.

#include "stdlib.h" 
#include "stdio.h"
#include "errno.h"
#include "unistd.h"
#include "string.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "signal.h"
#define PERM S_IRUSR|S_IWUSR


int main() {
void sig_usr3(int signo) {
    if(signo == SIGUSR1)
        printf("recieved sigal in process 3\npid is %d\n\n",getpid());
    exit(0);
}

void sig_usr4(int signo) {
    if(signo == SIGUSR1)
        printf("recieved sigal in process 4\npid is %d\n\n",getpid());
    exit(0);
}

void sig_usr5(int signo) {
    if(signo == SIGUSR1)
        printf("recieved sigal in process 5\npid is %d\n\n",getpid());
    exit(0);
}
    size_t msize;
    key_t shmid;
    pid_t *pid;
    msize = 6 * sizeof(pid_t);
    if( (shmid = shmget(IPC_PRIVATE, msize , PERM)) == -1 )   { 
        fprintf(stderr, "Share Memory Error:%s\n\a", strerror(errno));
        exit(1);
    }
    pid = shmat(shmid, 0, 0);
    memset(pid,0,msize);
    pid[0] =  getpid();
    //process 0
    if(fork() == 0) {
    //process 1 
        pid = shmat(shmid, 0, 0);
        pid[1] =  getpid();
        if(fork() == 0) {
            //process 5
            pid = shmat(shmid, 0, 0);
            pid[5] =  getpid();
            while(pid[3]==0)
                sleep(1);
            if((setpgid(pid[5],pid[3]))==-1)
                printf("pid5 setpgid error.\n");
            signal(SIGUSR1,sig_usr5);
            for(;;) 
                pause();
        }
        for(;;) 
            pause();
        exit(0);
      }

    if(fork() == 0) {
        //process 2
        pid = shmat(shmid, 0, 0);
        pid[2] =  getpid();
        if(fork() == 0) {
            //process 3
            pid = shmat(shmid, 0, 0);
            pid[3] =  getpid();
            if((setpgid(pid[3],pid[3]))==-1)
                printf("pid3 setpgid error.\n");
            if(fork() == 0) {
                //process 4
                pid = shmat(shmid, 0, 0);
                pid[4] =  getpid();
                if((setpgid(pid[4],pid[3]))==-1)
                    printf("pid4 setpgid error.\n");
                signal(SIGUSR1,sig_usr4);
                for(;;)
                    pause();
            }
            else {
                signal(SIGUSR1,sig_usr3);
                for(;;)  
                    pause();
            }
            for(;;)  
                sleep(100); 
    }

    if(getpid()==pid[0]) {
        int i,flag;
        while(!(pid[0]&&pid[1]&&pid[2]&&pid[3]&&pid[4]&&pid[5]))
            //wait for all process folking.
            sleep(1);

        for(i=0;i<6;i++)
            printf("process %d,pid:%d\n",i,pid[i]);
        kill(-pid[3],SIGUSR1);
    }
}

}