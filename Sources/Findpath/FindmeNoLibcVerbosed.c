#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

// function definitions to avoid using glibc 

// https://github.com/vendu/OS-Zero/blob/master/usr/lib/c/string.c


void printfb(const char * format, ...)
{
        #include <stdarg.h>
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
}

size_t
strlenb(const char *str)
{
    size_t len = 0;
    
    while (*str++) {
        len++;
    }
    
    return len;
}

#if ((defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 700))                 \
     || (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)))

size_t
strnlenb(const char *str, size_t maxlen)
{
    size_t len = 0;

    while (str[0] && (maxlen--)) {
        len++;
    }

    return len;
}

#endif

/* TESTED OK */

char *
strdupb(const char *str)
{
    size_t  len = strlenb(str);
    char   *buf = (len) ? malloc(len + 1) : NULL;

    if (buf) {
        memcpy(buf, str, len);
        buf[len] = '\0';
    } else {
        errno = ENOMEM;
    }

    return buf;
}

#if !defined(__GLIBC__)

#if ((defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)           \
     || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700))
char *
strndupb(const char *str, size_t maxlen)
{
    size_t  len = strlenb(str);
    size_t  sz = min(len, maxlen);
    char   *buf = (sz) ? malloc(sz + 1) : NULL;

    if (buf) {
        memcpy(buf, str, sz);
        buf[sz] = '\0';
    } else {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return buf;
}
#endif

#endif /* !__GLIBC__ */

void *
strchrb(const char *str,
       int ch)
{
    char *cptr = (char *)str;
    char  c = (char)ch;
    void *retval = NULL;
    
    while ((*cptr) && *cptr != c) {
        cptr++;
    }
    if (*cptr == c) {
        retval = cptr;
    }

    return retval;
}

char *
strcpyb(char *dest,
       const char *src)
{
    char *cptr = dest;

    while (*src) {
        *cptr++ = *src++;
    }
    *cptr = *src;

    return dest;
}

/* TESTED OK */

char *
  strncpyb(char *dest,
          const char *src,
          size_t n)
  {
      char *cptr = dest;
  
      if (n) {
          while ((*src) && (n--)) {
              *cptr++ = *src++;
          }
          if (n) {
              *cptr = *src;
          }
      }
  
      return dest;
  }

char *
strcatb(char *dest,
       const char *src)
{
    char *cptr = dest;

    while (*cptr) {
        cptr++;
    }
    while (*src) {
        *cptr++ = *src++;
    }
    *cptr = *src;

    return dest;
}

/* TESTED OK */
char *
strncatb(char *dest,
        const char *src,
        size_t n)
{
    char *cptr = dest;

    if (n) {
        while (*cptr) {
            cptr++;
        }
        while ((*src) && (n--)) {
            *cptr++ = *src++;
        }
        if (n) {
            *cptr = *src;
        }
    }

    return dest;
}

#if !defined(__GLIBC__)

/* TESTED OK */
int
strcmpb(const char *str1,
       const char *str2)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2 = (unsigned char *)str2;
    int            retval = 0;

    while ((*ucptr1) && *ucptr1 == *ucptr2) {
        ucptr1++;
        ucptr2++;
    }
    if (*ucptr1) {
        retval = (int)*ucptr1 - (int)*ucptr2;
    }

    return retval;
}

/* TESTED OK */
int
strncmpb(const char *str1,
        const char *str2,
        size_t n)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2 = (unsigned char *)str2;
    int            retval = 0;

    if (n) {
        while ((*ucptr1) && (*ucptr1 == *ucptr2) && (n--)) {
            ucptr1++;
            ucptr2++;
        }
        if (n) {
            retval = (int)*ucptr1 - (int)*ucptr2;
        }
    }

    return retval;
}

#endif /* !__GLIBC__ */

int
strcollb(const char *str1,
        const char *str2)
{
////      _exit(1);
    return 1;
}

char* strstrb(const char *str, const char *target) {
  char * stra = (char *)malloc(strlen(str) + 1);
  char * strcpyb(char *dest, const char *src);
  strcpyb(stra,str);
  if (!*target) return stra;
  char *p1 = (char*)str, *p2 = (char*)target;
  char *p1Adv = (char*)str;
  while (*++p2)
    p1Adv++;
  while (*p1Adv) {
    char *p1Begin = p1;
    p2 = (char*)target;
    while (*p1 && *p2 && *p1 == *p2) {
      p1++;
      p2++;
    }
    if (!*p2)
      return p1Begin;
    p1 = p1Begin + 1;
    p1Adv++;
  }
  return NULL;
}

#include<stdio.h>
#include <stdio.h>
#include <string.h>

char *
  getenvb(const char *envtarget)
  {
    extern char **environ;
    int i = 1;
    char *s = *environ;

    for (; s; i++) {
        if (s)
        {
//             printf("%s\n", s);
            char * pch;
            char * y = (char *)malloc(strlenb(s) + 1);
            char * strcpyb(char *dest, const char *src);
            strcpyb(y,s);
            pch = strtok (y,"=");
            while (pch != '\0')
                {
                    char * NAME = pch;
                    const char * STTR = envtarget;
//                     printf("trying \"%s\"\n", y);
                    int strcmpC(const char *str1, const char *str2)
                    {
                        unsigned char *ucptr1 = (unsigned char *)str1;
                        unsigned char *ucptr2 = (unsigned char *)str2;
                        int            retval = 0;

                        while ((*ucptr1) && *ucptr1 == *ucptr2) {
                            ucptr1++;
                            ucptr2++;
                        }
                        if (*ucptr1) {
                            retval = (int)*ucptr1 - (int)*ucptr2;
                        }

                        return retval;
                    }
                    if (strcmpC(NAME,STTR) == 0 )
                        {
//                             printf("  MATCH FOUND=\"%s\"\n", y);
                            char * pch = strtok (NULL, "=");
                            char * VALUE = pch;
//                             printf ("VALUE = %s\n",VALUE);
                            return VALUE;
                        }
                    if (strcmpC(NAME,STTR) != 0 )
                        {
//                             printf("  \"%s\" did not match \"%s\"\n", NAME,STTR);
                        }
                    break;
                }
            free(y);
        }
        s = *(environ+i);
    }
free(s);
return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

char *
resolve(char * path, char * save_to_variable)
  {
   printf("called resolve()\n");
        char *cptr = save_to_variable;

      if(!access(path, F_OK)) {
        } else {
        return(0);
        }

    char * pathb = (char *)malloc(strlen(path) + 1);
    char * strcpyb(char *dest, const char *src);
    strcpyb(pathb,path);
    char save_pwd[PATH_MAX];
    getcwd(save_pwd, sizeof(save_pwd));
    char path_separator='/';
    char relpathdot_separator[4]="/./";
    char relpathdotdor_separator[5]="/../";
    char newpathb[PATH_MAX+256];
    char newpathc[PATH_MAX+256];
    char linkb[PATH_MAX+256];
    char linkd[PATH_MAX+256];
    char tmp_pwd[PATH_MAX];
    char current_pwd[PATH_MAX];
    getcwd(current_pwd, sizeof(current_pwd));
        #include <sys/types.h>
        #include <sys/stat.h>
    char* resolvedir(const char * pathb)
    {
        chdir(pathb);
        getcwd(tmp_pwd, sizeof(tmp_pwd));
//        printf("%s points to %s\n\n", pathb, tmp_pwd);
        chdir(current_pwd);
        return tmp_pwd;
    }
    char* resolvefile(char * pathb)
    {
        strncpyb(linkb, pathb, sizeof(linkb));
        linkb[sizeof(linkb)-1]=0;
        strncpyb(linkd, pathb, sizeof(linkb));
        linkb[sizeof(linkb)-1]=0;
        dirname(linkd);
        strncatb(linkd, "/", sizeof(linkd));
        linkd[sizeof(linkd)-1]=0;
        chdir(linkd);
        getcwd(tmp_pwd, sizeof(tmp_pwd));
        strncatb(tmp_pwd, "/", sizeof(tmp_pwd));
        tmp_pwd[sizeof(tmp_pwd)-1]=0;
        strncpyb(linkb, basename(pathb), sizeof(linkb));
        linkb[sizeof(linkb)-1]=0;
        strncatb(tmp_pwd, linkb, sizeof(tmp_pwd));
        tmp_pwd[sizeof(tmp_pwd)-1]=0;
//        printf("%s points to %s\n\n", pathb, tmp_pwd);
        chdir(current_pwd);
        return tmp_pwd;
    }
    #include <sys/types.h>
    #include <sys/stat.h>
    char * getlink(const char * link)
    {
        struct stat p_statbuf;
        if (lstat(link,&p_statbuf)==0) {
            printf("%s type is %i\n",link, S_ISLNK(p_statbuf.st_mode));
            if (S_ISLNK(p_statbuf.st_mode)==1)
            {
            printf("%s is symbolic link \n", link);
            } else
            {
            printf("%s is not symbolic link \n", link);
                return 0;
            }
        }
        struct stat sb;
        char *linkname;
        ssize_t r;

        if (lstat(link, &sb) == -1)
        {
                _exit(EXIT_FAILURE);
        }

        linkname = malloc(sb.st_size + 1);
            if (linkname == NULL)
            {
                _exit(EXIT_FAILURE);
            }

        r = readlink(link, linkname, sb.st_size + 1);

        if (r < 0)
        {
                _exit(EXIT_FAILURE);
        }

        if (r > sb.st_size)
        {
                _exit(EXIT_FAILURE);
        }

        linkname[sb.st_size] = '\0';

       printf("\"%s\" points to '%s'\n", link, linkname);

        path = linkname;
        char * checkifsymlink(const char * tlink)
        {
            struct stat p_statbuf;
            if (lstat(tlink,&p_statbuf)==0)
            {
                printf("%s type is %i\n",tlink, S_ISLNK(p_statbuf.st_mode));
                if (S_ISLNK(p_statbuf.st_mode)==1)
                {
                printf("%s is symbolic link \n", tlink);
                printf("called getlink()\n");
                    getlink(tlink);
                } else
                {
                printf("%s is not symbolic link \n", tlink);
                    return 0;
                }
            }
        return 0;
        }
       printf("called checkifsymlink()\n");
        checkifsymlink(path);
        return 0;
    }
   printf("called getlink()\n");
    getlink(path);
    char * testtype(const char * patha)
    {
        int is_regular_file(const char *patha)
        {
            struct stat path_stat;
            stat(patha, &path_stat);
            return S_ISREG(path_stat.st_mode);
        }
        int isDirectory(const char *patha)
        {
            struct stat statbuf;
            if (stat(patha, &statbuf) != 0)
                return 0;
            return S_ISDIR(statbuf.st_mode);
        }
        if (is_regular_file(patha)==1)
        {
           printf("%s is file \n", patha);
            if (path[0]==path_separator)
            {
                if ( strstrb(path, relpathdot_separator ))
                {
                   printf("    %s is an absolute path which contains a dot relative path\n", path);
                   printf("called resolvefile()\n");
                    return resolvefile(path);
                } else if ( strstrb(path, relpathdotdor_separator ))
                {
                   printf("    %s is an absolute path which contains a dot dot relative path\n", path);
                   printf("called resolvefile()\n");
                    resolvefile(path);
                    return 0;
                } else
                {
                   printf("    %s is an absolute path with no relative paths\n", path);
                    return path;
                }
                return 0;
            } else if ( strchrb(path, path_separator ))
            {
               printf("    %s is a relative path\n", path);
                strncpyb(newpathb, current_pwd, sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
                strncatb(newpathb, "/", sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
                strncatb(newpathb, path, sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
               printf("called resolvefile()\n");
                resolvefile(newpathb);
                return 0;
            } else
            {
               printf("could not determine path type of %s\n", path);
        //         return 1;
            }
        } else if (isDirectory(patha)==1)
        {
           printf("%s is a directory \n", patha);
            if (path[0]==path_separator)
            {
                if ( strstrb(path, relpathdot_separator ))
                {
                   printf("    %s is an absolute path which contains a dot relative path\n", path);
                   printf("called resolvedir()\n");
                    resolvedir(path);
                } else if ( strstrb(path, relpathdotdor_separator ))
                {
                   printf("    %s is an absolute path which contains a dot dot relative path\n", path);
                   printf("called resolvedir()\n");
                    resolvedir(path);
                } else
                {
                   printf("    %s is an absolute path with no relative paths\n", path);
                    return path;
                }
                return 0;
            } else if ( strchrb(path, path_separator ))
            {
               printf("    %s is a relative path\n", path);
               printf("    strncpyb(%s, %s, sizeof(%s));\n", newpathc, current_pwd, newpathc);
                strncpyb(newpathc, current_pwd, sizeof(newpathc));
               printf("    newpath2[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               printf("    strncatb(%s, %s, sizeof(%s));\n", newpathc, "/", newpathc);
                strncatb(newpathc, "/", sizeof(newpathc));
               printf("    newpathc[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               printf("    strncatb(%s, %s, sizeof(%s));\n", newpathc, path, newpathc);
                strncatb(newpathc, path, sizeof(newpathc));
               printf("    newpathc[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               printf("called resolvedir()\n");
                resolvedir(newpathc);
                return 0;
            } else
            {
               printf("could not determine path type of %s\n", path);
        //         return 1;
            }
        }
        return 0;
    }
   printf("called testtype()\n");
    save_to_variable = testtype(path);
    while (*cptr) {
        cptr++;
    }
    while (*save_to_variable) {
        *cptr++ = *save_to_variable++;
    }
    *cptr = *save_to_variable;

    return cptr;
  }


// "look deep into yourself, Clarice"  -- Hanibal Lector
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

char findyourself_save_pwd[PATH_MAX];
char findyourself_save_argv0[PATH_MAX];
char findyourself_save_path[PATH_MAX];
char findyourself_path_separator='/';
char findyourself_path_separator_as_string[2]="/";
char findyourself_path_list_separator[8]=":";  // could be ":; "
char findyourself_debug=2;
char findyourself_verbose=1;

int findyourself_initialized=0;

void findyourself_init(char *argv0)
{

if(findyourself_verbose) printf("  getcwd(%s,sizeof(%s));\n", findyourself_save_pwd, findyourself_save_pwd);
  getcwd(findyourself_save_pwd, sizeof(findyourself_save_pwd));
if(findyourself_verbose) printf("  getcwd(%s,sizeof(%s));\n", findyourself_save_pwd, findyourself_save_pwd);

if(findyourself_verbose) printf("  strncpyb(%s, %s, sizeof(%s));\n", findyourself_save_argv0, argv0, findyourself_save_argv0);
  strncpyb(findyourself_save_argv0, argv0, sizeof(findyourself_save_argv0));
if(findyourself_verbose) printf("  findyourself_save_argv0[sizeof(%s)-1)=0;\n", findyourself_save_argv0);
  findyourself_save_argv0[sizeof(findyourself_save_argv0)-1]=0;

if(findyourself_verbose) printf("  strncpyb(%s, getenvb(\"PATH\"), sizeof(%s));\n", findyourself_save_path, findyourself_save_path);
  strncpyb(findyourself_save_path, getenvb("PATH"), sizeof(findyourself_save_path));
if(findyourself_verbose) printf("  findyourself_save_path[sizeof(%s)-1)=0;\n", findyourself_save_path);
  findyourself_save_path[sizeof(findyourself_save_path)-1]=0;
if(findyourself_verbose) printf("  findyourself_initialized=1\n");
  findyourself_initialized=1;
}


int find_yourself(char *result, size_t size_of_result)
{
  char newpath[PATH_MAX+256];
  char newpath2[PATH_MAX+256];

if(findyourself_verbose) printf("  assert(%i)\n", findyourself_initialized);
  assert(findyourself_initialized);
if(findyourself_verbose) printf("  result[0]=0\n");
  result[0]=0;
if(findyourself_verbose) printf("  if(%s==%c) {\n",findyourself_save_argv0, findyourself_path_separator);
  if(findyourself_save_argv0[0]==findyourself_path_separator) {
    if(findyourself_debug) printf("     absolute path\n");
if(findyourself_verbose) printf("     resolve(%s);\n",findyourself_save_argv0);
     resolve(findyourself_save_argv0, newpath);
     if(findyourself_debug) printf("     newpath=\"%s\"\n", newpath);
if(findyourself_verbose) printf("     if(!access(%s, F_OK)) {\n", newpath);
     if(!access(newpath, F_OK)) {
if(findyourself_verbose) printf("        strncpyb(%s, %s, %i);\n", result, newpath, size_of_result);
        strncpyb(result, newpath, size_of_result);
if(findyourself_verbose) printf("        result[%i-1]=0;\n", size_of_result);
        result[size_of_result-1]=0;
if(findyourself_verbose) printf("        return(0);\n");
        return(0);
     } else {
if(findyourself_verbose) printf("     } else {\n");
if(findyourself_verbose) printf("    perror(\"access failed 1\");\n");
    perror("access failed 1");
if(findyourself_verbose) printf("      }\n");
      }
  } else if( strchrb(findyourself_save_argv0, findyourself_path_separator )) {
if(findyourself_verbose) printf("  } else if( strchrb(%s, %c )) {\n",findyourself_save_argv0, findyourself_path_separator );
    if(findyourself_debug) printf("    relative path to pwd\n");
if(findyourself_verbose) printf("    strncpyb(%s, %s, sizeof(%s));\n", newpath2, findyourself_save_pwd, newpath2);
    strncpyb(newpath2, findyourself_save_pwd, sizeof(newpath2));
if(findyourself_verbose) printf("    newpath2[sizeof(%s)-1]=0;\n", newpath2);
    newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("    strncatb(%s, %s, sizeof(%s));\n", newpath2, findyourself_path_separator_as_string, newpath2);
    strncatb(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
if(findyourself_verbose) printf("    newpath2[sizeof(%s)-1]=0;\n", newpath2);
    newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("    strncatb(%s, %s, sizeof(%s));\n", newpath2, findyourself_save_argv0, newpath2);
    strncatb(newpath2, findyourself_save_argv0, sizeof(newpath2));
if(findyourself_verbose) printf("    newpath2[sizeof(%s)-1]=0;\n", newpath2);
    newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("    resolve(%s);\n",newpath2);
    resolve(newpath2, newpath);
    if(findyourself_debug) printf("    newpath=\"%s\"\n", newpath);
if(findyourself_verbose) printf("    if(!access(%s, F_OK)) {\n", newpath);
    if(!access(newpath, F_OK)) {
if(findyourself_verbose) printf("        strncpyb(%s, %s, %i);\n", result, newpath, size_of_result);
        strncpyb(result, newpath, size_of_result);
if(findyourself_verbose) printf("        result[%i-1]=0;\n", size_of_result);
        result[size_of_result-1]=0;
if(findyourself_verbose) printf("        return(0);\n");
        return(0);
     } else {
if(findyourself_verbose) printf("     } else {\n");
if(findyourself_verbose) printf("    perror(\"access failed 2\");\n");
    perror("access failed 2");
if(findyourself_verbose) printf("      }\n");
      }
  } else {
if(findyourself_verbose) printf("  } else {\n");
    if(findyourself_debug) printf("    searching $PATH\n");
    char *saveptr;
    char *pathitem;
if(findyourself_verbose) printf("    for(pathitem=strtok_r(%s, %s,  %s); %s; pathitem=strtok_r(NULL, %s, %s) ) {;\n", findyourself_save_path, findyourself_path_list_separator,  &saveptr, pathitem, findyourself_path_list_separator, &saveptr);
    for(pathitem=strtok_r(findyourself_save_path, findyourself_path_list_separator,  &saveptr); pathitem; pathitem=strtok_r(NULL, findyourself_path_list_separator, &saveptr) ) {
       if(findyourself_debug>=2) printf("       pathitem=\"%s\"\n", pathitem);
if(findyourself_verbose) printf("       strncpyb(%s, %s, sizeof(%s));\n", newpath2, pathitem, newpath2);
       strncpyb(newpath2, pathitem, sizeof(newpath2));
if(findyourself_verbose) printf("       newpath2[sizeof(%s)-1]=0;\n", newpath2);
       newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("       strncatb(%s, %s, sizeof(%s));\n", newpath2, findyourself_path_separator_as_string, newpath2);
       strncatb(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
if(findyourself_verbose) printf("       newpath2[sizeof(%s)-1]=0;\n", newpath2);
       newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("       strncatb(%s, %s, sizeof(%s));\n", newpath2, findyourself_save_argv0, newpath2);
       strncatb(newpath2, findyourself_save_argv0, sizeof(newpath2));
if(findyourself_verbose) printf("       newpath2[sizeof(%s)-1]=0;\n", newpath2);
       newpath2[sizeof(newpath2)-1]=0;
if(findyourself_verbose) printf("       resolve(%s);\n",newpath2);
       resolve(newpath2, newpath);
       if(findyourself_debug) printf("       newpath=\"%s\"\n", newpath);
if(findyourself_verbose) printf("       if(!access(%s, F_OK)) {\n", newpath);
       if(!access(newpath, F_OK)) {
if(findyourself_verbose) printf("          strncpyb(%s, %s, %i);\n", result, newpath, size_of_result);
          strncpyb(result, newpath, size_of_result);
if(findyourself_verbose) printf("          result[%i-1]=0;\n", size_of_result);
          result[size_of_result-1]=0;
if(findyourself_verbose) printf("          return(0);\n");
if(findyourself_verbose) printf("      }\n");
          return(0);
      }
if(findyourself_verbose) printf("    }\n");
    } // end for
if(findyourself_verbose) printf("    perror(\"access failed 3\");\n");
    perror("access failed 3");
if(findyourself_verbose) printf("  }\n");
  } // end else
  // if we get here, we have tried all three methods on argv[0] and still haven't succeeded.   Include fallback methods here.
if(findyourself_verbose) printf("  return(1);\n");
if(findyourself_verbose) printf("}\n");
  return(1);
}

int
main (int argc, char **argv)
{
  findyourself_init(argv[0]);
  char newpath[PATH_MAX];
  find_yourself(newpath, PATH_MAX);
    int
    strcmpb(const char *str1,
        const char *str2)
    {
        unsigned char *ucptr1 = (unsigned char *)str1;
        unsigned char *ucptr2 = (unsigned char *)str2;
        int            retval = 0;

        while ((*ucptr1) && *ucptr1 == *ucptr2) {
            ucptr1++;
            ucptr2++;
        }
        if (*ucptr1) {
            retval = (int)*ucptr1 - (int)*ucptr2;
        }

        return retval;
    }

  if(1 || strcmpb(argv[0],newpath)) { printf("  findyourself=\"%s\"\n", newpath); }
  char *dummy  = strdupb( newpath );
  char *dname = dirname( dummy );
  printf("  Directory=\"%s\"\n", dname);
  chdir(dname); // changes into the directory of this application
}
