#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
    int pid = getpid();
    int ppid = getppid();

    printf("my pid    : %d\n", pid);
    printf("my ppid   : %d\n", ppid);

    exit(0);
}