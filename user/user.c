#include "user.h"
#include "kernel/proc.h"

int getpinfo(struct pinfo *info){
    return syscall(SYS_getpinfo, info, 0, 0, 0, 0, 0);
}