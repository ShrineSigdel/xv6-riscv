
# Off / Halt

Simple documentation for the `off` user program and the kernel `sys_halt` implementation.

---

## 1. `off` — Shutdown the machine

### Overview
The `off` command is a tiny user program that requests the kernel to shut down the virtual machine. On QEMU (the usual xv6 test environment) this is implemented by a kernel syscall that signals the QEMU virt test finisher.

### Usage
Run `off` from the shell. The program prints a shutdown message and the machine stops (QEMU exits).

### User program
The user-side program lives in `user/off.c` and simply calls the syscall wrapper then exits:

```c
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
		halt();
		exit(0); // never reached
}
```

`halt()` is a small user stub that invokes the kernel syscall which performs the actual shutdown.

### Kernel implementation — `sys_halt`
The kernel implements the shutdown in `kernel/sysproc.c`. A minimal implementation prints a message and writes a magic value to the QEMU virt test finisher address (`VIRT_TEST`) so QEMU will stop:

```c
uint64
sys_halt(void)
{
	printf("shutting down...\n");

	/* Fallback: QEMU virt test finisher */
	volatile uint *p = (volatile uint *)VIRT_TEST;
	*p = 0x5555;

	return 0; // not reached
}
```

Writing `0x5555` to `VIRT_TEST` is the mechanism used by the xv6 test harness to signal a successful virtual poweroff when running under QEMU's `virt` machine.

### Files changed / touched
- `user/off.c` — user program that calls `halt()`
- `user/usys.pl` / `user/usys.S` — ensure the `halt` syscall stub exists
- `kernel/sysproc.c` — contains `sys_halt()` implementation

### Sample output
```
$ off
shutting down...
```

On QEMU this will cause the emulator to stop and return control to the host shell.

### Notes
- On non-QEMU platforms or different QEMU machine types the shutdown mechanism may differ; `VIRT_TEST` is a QEMU-specific testing interface used by the xv6 test harness.
- If `off` doesn't stop your emulator, check that the build and run options target the `virt` machine and that `VIRT_TEST` is defined in your kernel headers.

### Kernel notes (what I changed)
- `memlayout.h`: `VIRT_TEST` is defined as the QEMU virt test finisher address:

	```c
	// QEMU virt machine test finisher (SiFive Test Device)
	// mapped at 0x100000 in the virt machine memory map
	// write 0x5555 to trigger a clean shutdown (FINISHER_PASS)
	// write 0x3333 to trigger a reboot
	// write 0x1234 to trigger a test failure (FINISHER_FAIL)
	#define VIRT_TEST   0x100000L
	```

- `vm.c`: the test device region is mapped into the kernel page table so the kernel can write to it:

	```c
		// test region for lazy allocation.
		kvmmap(kpgtbl, VIRT_TEST, VIRT_TEST, PGSIZE, PTE_R | PTE_W);
	```

These changes ensure the kernel can safely write the finisher magic value at `VIRT_TEST` to signal QEMU to exit.
