// Copyright 2015 by Mark Whitis.  License=MIT style
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

// "look deep into yourself, Clarice"  -- Hanibal Lector
char findyourself_save_pwd[PATH_MAX];
char findyourself_save_argv0[PATH_MAX];
char findyourself_save_path[PATH_MAX];
char findyourself_path_separator='/';
char findyourself_path_separator_as_string[2]="/";
char findyourself_path_list_separator[8]=":";  // could be ":; "
char findyourself_debug=0;

int findyourself_initialized=0;

void findyourself_init(char *argv0)
{

  getcwd(findyourself_save_pwd, sizeof(findyourself_save_pwd));

  strncpy(findyourself_save_argv0, argv0, sizeof(findyourself_save_argv0));
  findyourself_save_argv0[sizeof(findyourself_save_argv0)-1]=0;

  strncpy(findyourself_save_path, getenv("PATH"), sizeof(findyourself_save_path));
  findyourself_save_path[sizeof(findyourself_save_path)-1]=0;
  findyourself_initialized=1;
}


int find_yourself(char *result, size_t size_of_result)
{
  char newpath[PATH_MAX+256];
  char newpath2[PATH_MAX+256];

  assert(findyourself_initialized);
  result[0]=0;

  if(findyourself_save_argv0[0]==findyourself_path_separator) {
    if(findyourself_debug) printf("  absolute path\n");
     realpath(findyourself_save_argv0, newpath);
     if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
     if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 1");
      }
  } else if( strchr(findyourself_save_argv0, findyourself_path_separator )) {
    if(findyourself_debug) printf("  relative path to pwd\n");
    strncpy(newpath2, findyourself_save_pwd, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findyourself_save_argv0, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    realpath(newpath2, newpath);
    if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
    if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 2");
      }
  } else {
    if(findyourself_debug) printf("  searching $PATH\n");
    char *saveptr;
    char *pathitem;
    for(pathitem=strtok_r(findyourself_save_path, findyourself_path_list_separator,  &saveptr); pathitem; pathitem=strtok_r(NULL, findyourself_path_list_separator, &saveptr) ) {
       if(findyourself_debug>=2) printf("pathitem=\"%s\"\n", pathitem);
       strncpy(newpath2, pathitem, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findyourself_save_argv0, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       realpath(newpath2, newpath);
       if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
      if(!access(newpath, F_OK)) {
          strncpy(result, newpath, size_of_result);
          result[size_of_result-1]=0;
          return(0);
      } 
    } // end for
    perror("access failed 3");

  } // end else
  // if we get here, we have tried all three methods on argv[0] and still haven't succeeded.   Include fallback methods here.
  return(1);
}

int main(int argc, char **argv)
{
  findyourself_init(argv[0]);

  char newpath[PATH_MAX];
  find_yourself(newpath, sizeof(newpath));
  if(1 || strcmp(argv[0],newpath)) { printf("  findyourself=\"%s\"\n", newpath); }
  char *dummy  = strdup( newpath );
  char *dname = dirname( dummy );
  printf("  Directory=\"%s\"\n", dname);
  chdir(dname); // changes into the directory of this application

  char **args = argv + 1; // credit: flawless_snowflake from kik messenger

  execv(argv[1], args); // executes any executable file  (CANNOT USE $PATH SO NEEDS TO BE RELATIVE OR FULL PATH EXCEPT WHEN EXECUTING FILES IN EXECUTABLE DIR FOR EXAMPLE BASH WILL BE THE SAME AS $AppLocation/BASH ) contained in the folder that this application is contained in   // credit: flawless_snowflake from kik messenger
  free( dummy );
  sleep(0);  /* in case run from desktop */
  return 0;
}