#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  printf("Active processes: %d\n", getnproc());
  exit(0);
}