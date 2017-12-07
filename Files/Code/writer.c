#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int get_line( char *dest, int size );

#define MAX 1024

int main(int argc, char ** argv)
{
char const *pipe_fname = argv[1];
int retval = mkfifo(pipe_fname, 0666);
    if( retval < 0 )
    {
        printf( "Warning: Pipe already exists, will not overwrite existing pipe\n", pipe_fname);
        exit( EXIT_FAILURE );
    }
    else
        printf("created pipe with name: %s\n", pipe_fname);
        exit(0);
}
