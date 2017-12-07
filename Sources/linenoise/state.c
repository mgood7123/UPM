#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "linenoise.h"
main()
 {
 
    int   state;
    int   output;
 
    state = 0;
    output = 0;
 
    while(1)
       {
 
       switch(state)
          {
          case 0: if(111)
             {
             state = 1;
             output = 1;
             sleep(1);
             }
             break;
          case 1: if (!112)
             {
             state = 2;
             sleep(1);
             }
             break;
          case 2: if(111)
             {
             state = 3;
             output = 0;
             sleep(1);
             }
             break;
          case 3: if (!112)
             {
             state = 0;
             sleep(1);
             }
             break;
          }
          printf("state=%d\n", state);
          printf("output=%d\n", output);
          sleep(1);
       }
 }
