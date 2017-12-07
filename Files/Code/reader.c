#include <fcntl.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
/* For simplicity, all error checking has been left out */

	 #include <sys/types.h>
	 #include <sys/stat.h>

#define INPUT_BUFFER_SZ 1024

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage: %s pipe_filename\n", argv[0]);
		return 1;
	}

	const char *pipe_filename = argv[1];
//        fprintf(stdout, "pipe-->%s\n", pipe_filename);
	if(mkfifo(pipe_filename, 0666) != 0 && errno != EEXIST) {
		fprintf(stderr, "error creating fifo, errno=%d\n", errno);
		return 1;
	}

	int abort = 0;
	int bytes_read = 0;
	char buffer[INPUT_BUFFER_SZ];

	while(!abort) {
printf("test1\n"); system("cat ./stout & "); printf("test2\n");
        
// 		if(poll(&poll_fds, 1, 1000) == 1) {
// 			if(poll_fds.revents & (POLLERR | POLLNVAL)) {
// 				poll_fds.fd = 0;
// 				abort = 1;
// 			}
// 
// 			'if(poll_fds.revents & POLLIN) {
//                             printf("POLLIN Triggered\n");
// 				bytes_read = read(poll_fds.fd, buffer, INPUT_BUFFER_SZ);
// 				if(bytes_read == 0) {
// 					abort = 1;
// 				} else {
// //                                       fprintf(stdout, "pipe-->%s\n", pipe_filename);
//                                        system("cat ./stout");
// 				}
// //			}
// //
// //			if(poll_fds.revents & POLLHUP) { printf("POLLHUP Triggered\n");
// //				if(poll_fds.fd) {
// //					close(poll_fds.fd);
// //				}
// //				poll_fds.fd = open(pipe_filename, O_RDONLY);
// 			}
// 		}
 	}
// 
//	if(poll_fds.fd) {
//		close(poll_fds.fd);
//	}
	return 0;
}