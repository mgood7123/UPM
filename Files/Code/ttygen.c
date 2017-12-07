#define _XOPEN_SOURCE 600 
#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 

int main(void) 
{ 
int fdm; 
int rc; 

// Display /dev/pts 
system("ls -l /dev/pts"); 

fdm = posix_openpt(O_RDWR); 
if (fdm < 0) 
{ 
fprintf(stderr, "Error %d on posix_openpt()\n", errno); 
return 1; 
} 

rc = grantpt(fdm); 
if (rc != 0) 
{ 
fprintf(stderr, "Error %d on grantpt()\n", errno); 
return 1; 
} 

rc = unlockpt(fdm); 
if (rc != 0) 
{ 
fprintf(stderr, "Error %d on unlockpt()\n", errno); 
return 1; 
} 

// Display the changes in /dev/pts 
system("ls -l /dev/pts"); 

printf("The slave side is named : %s\n", ptsname(fdm)); 

return 0; 
} // main