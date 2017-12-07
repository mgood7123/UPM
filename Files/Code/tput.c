#include <unistd.h>
#include <stdio.h>
main() {
    printf("\0337");
    printf("\33[1;1H");
    printf("\33[K");
    printf("test");
    printf("\0338");
}
