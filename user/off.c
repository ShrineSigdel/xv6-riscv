#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
    halt();
    exit(0); // never reached
}