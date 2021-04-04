#include <stdio.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname n;

    uname(&n);

    printf("sysname : %s\n", n.sysname);
    printf("nodename : %s\n", n.nodename);
    printf("release : %s\n", n.release);
    printf("version : %s\n", n.version);
    printf("machine : %s\n", n.machine);

    return 0;
}