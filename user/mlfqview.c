#include "kernel/types.h"   // user-space types like int, uint, etc.
#include "user/user.h"      // standard user functions (printf, exit, malloc, etc.)
#include "kernel/fcntl.h"   // optional: for open/read/write
#include "kernel/stat.h"    // optional: for file stats


// user/mlfqview.c
struct pinfo {
    int pid;
    int priority;
    int curr_ticks;
    int ticks[4];
};

int main() {
    struct pinfo info[64]; // enough to hold all processes
    int n = getpinfo(info);

    if(n < 0) {
        printf("getpinfo syscall failed\n");
        exit(1);
    }

    printf("PID  PRIORITY  CURR_TICKS  TICKS[0-3]\n");
    for(int i = 0; i < n; i++){
        printf("%d     %d        %d     %d %d %d %d\n",
               info[i].pid, info[i].priority, info[i].curr_ticks,
               info[i].ticks[0], info[i].ticks[1],
               info[i].ticks[2], info[i].ticks[3]);
    }
    exit(0);
}