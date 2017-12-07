// Origin: https://stackoverflow.com/questions/1516122/how-to-capture-controld-signal/1516177
// http://codepad.org/ct9lA7uK    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <findme.h>
        if(argc < 2) {
		fprintf(stderr, "usage: %s pipe_filename\n", argv[0]);
        fprintf(stderr, "example:\nfrom terminal 1: %s pipe\nfrom terminal 2: echo hi > ./pipe\nfrom terminal 2: killall %s\n", argv[0], argv[0]);
		return 1;
	}

	const char *pipe_filename = argv[1];
//        fprintf(stdout, "pipe-->%s\n", pipe_filename);
	if(mkfifo(pipe_filename, 0666) != 0 && errno != EEXIST) {
		fprintf(stderr, "error creating fifo, errno=%d\n", errno);
		return 1;
	}

#define STDIN_FILENO 0
#define INPUT_BUFFER_SZ 1024

struct termios org_opts;

/** Select to check if stdin has pending input */
int pending_input(void) {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
//  FD_SET(pipe_filename, &fds); //STDIN_FILENO is 0
  select(STDIN_FILENO, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}

/** Input terminal mode; save old, setup new */
void setup_terminal(void) {
  struct termios new_opts;
  tcgetattr(STDIN_FILENO, &org_opts);
  memcpy(&new_opts, &org_opts, sizeof(new_opts));
  new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ISIG | ICRNL);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}

/** Shutdown terminal mode */
void reset_terminal(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
}

/** Return next input or -1 if none */
int next_input(void) {
  if (!pending_input())
    return -1;
  int rtn = fgetc(stdin);
  return(rtn);
}
	int abort = 0;
	int bytes_read = 0;
	char buffer[INPUT_BUFFER_SZ];

setup_terminal();
  printf("\033[2J\033[0;0fPress F2 to quit...\n\033[s");
  for (;;) {
    system("cat ./stout");
    printf("\0337");
    printf("\033[1;1H");
    printf("\033[K");
    printf("Press Ctrl+S to Stop output and Ctrl+Q to resume output...");
    printf("\0338");
    fflush(stdout);
//    system("tput sc ; tput cup 0 0 ; tput el ; printf Press Ctrl+S to Stop output and Ctrl+Q to resume output... ; tput rc");
//     int key = next_input();
//     if (key != -1) {
//       if ((key == 81)) {
//         printf("\nNormal exit\n");
//         kill(0,SIGTERM);
//         break;
//       }
//     }
  }

  reset_terminal();
  return 0;
}
