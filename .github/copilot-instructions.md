# xv6-riscv Copilot Instructions

## Project Overview

xv6-riscv is an educational operating system from MIT's 6.1810 course (formerly 6.828). It's a re-implementation of Unix Version 6 for the RISC-V architecture, designed for teaching OS concepts. The codebase is intentionally minimal (~11K lines) to be understandable while demonstrating core OS mechanisms.

## Build & Test

### Building
```bash
make qemu TOOLPREFIX=riscv64-linux-gnu-
```
- Compiles kernel and user programs
- Launches QEMU with the built kernel
- Exit QEMU: `Ctrl+A` then `X`

### Building without running
```bash
make TOOLPREFIX=riscv64-linux-gnu-
```

### Testing
```bash
# Run all tests
python3 test-xv6.py usertests

# Run specific test
python3 test-xv6.py <test_regex>

# Quick test mode
python3 test-xv6.py -q usertests
```

The test script boots xv6 in QEMU and runs usertests. Test regex filters which tests to run.

### Debugging
```bash
# Terminal 1: Start QEMU with GDB stub
make qemu-gdb TOOLPREFIX=riscv64-linux-gnu-

# Terminal 2: Connect GDB
riscv64-linux-gnu-gdb
```

The `.gdbinit` file is auto-generated from `.gdbinit.tmpl-riscv` with the correct port.

### Cleaning
```bash
make clean
```

## Architecture

### Directory Structure

- **kernel/** - Kernel code (~6K lines)
  - Process management: `proc.c`, `proc.h`, `swtch.S`
  - Memory: `vm.c`, `kalloc.c`, `memlayout.h`
  - System calls: `syscall.c`, `sysproc.c`, `sysfile.c`
  - File system: `fs.c`, `file.c`, `bio.c`, `log.c`
  - Traps/interrupts: `trap.c`, `trampoline.S`, `kernelvec.S`
  - Devices: `uart.c`, `console.c`, `virtio_disk.c`, `plic.c`
  - Entry: `entry.S`, `start.c`, `main.c`

- **user/** - User-space programs and libraries
  - Programs: `*.c` files (cat, ls, sh, grep, etc.)
  - User library: `ulib.c`, `umalloc.c`, `printf.c`
  - Syscall stubs: Generated from `usys.pl` into `usys.S`
  - Linker script: `user.ld`

- **mkfs/** - File system image creator
  - `mkfs.c` builds `fs.img` containing user programs

### Key Subsystems

**Process Model**
- Kernel uses struct proc (in `proc.h`) to track processes
- Each process has: page table, kernel stack, trapframe, context
- Scheduler uses round-robin in `scheduler()` function
- Context switching via `swtch()` assembly routine
- Process creation: `fork()` copies parent, `exec()` loads new program

**Memory Management**
- Physical memory allocator: `kalloc.c` (page-granularity free list)
- Virtual memory: `vm.c` handles page tables (RISC-V Sv39 3-level)
- User memory grows with `sbrk()` syscall
- Kernel identity-mapped, each process has separate page table

**File System**
- Simple Unix-like FS with inodes, directories, paths
- Block layer: `bio.c` (buffer cache for disk blocks)
- Logging: `log.c` (crash recovery via write-ahead log)
- File descriptor layer: `file.c`, `sysfile.c`
- On-disk layout: superblock, log, inode blocks, bitmap, data blocks

**Traps & System Calls**
- User→kernel transitions via `ecall` instruction (RISC-V)
- `trampoline.S` handles mode switches (saves/restores registers)
- Trap handler: `usertrap()` in `trap.c` dispatches to handlers
- Timer interrupts for preemption, device interrupts via PLIC

## Adding a New System Call

System calls require changes in multiple places. Follow this exact sequence:

### 1. Define syscall number in `kernel/syscall.h`
```c
#define SYS_mysyscall  22  // Use next available number
```

### 2. Add syscall stub generation in `user/usys.pl`
```perl
entry("mysyscall");  // Add at end of file
```

### 3. Declare prototype in `user/user.h`
```c
int mysyscall(int arg);  // Match your signature
```

### 4. Implement kernel function in `kernel/sysproc.c` or `kernel/sysfile.c`
```c
uint64
sys_mysyscall(void)
{
  int n;
  argint(0, &n);  // Extract argument from trapframe
  // Implementation here
  return result;
}
```

Use arg functions to extract arguments:
- `argint(n, &i)` - integer argument
- `argaddr(n, &p)` - pointer/address argument  
- `argstr(n, buf, max)` - string argument
- `argfd(n, &fd, &f)` - file descriptor argument

### 5. Add extern declaration in `kernel/syscall.c`
```c
extern uint64 sys_mysyscall(void);
```

### 6. Add to syscall table in `kernel/syscall.c`
```c
static uint64 (*syscalls[])(void) = {
  // ... existing entries ...
  [SYS_mysyscall]   sys_mysyscall,
};
```

### 7. Rebuild everything
```bash
make clean
make qemu TOOLPREFIX=riscv64-linux-gnu-
```

The `usys.pl` Perl script generates `usys.S` assembly stubs. Each stub loads the syscall number into register `a7` and executes `ecall`, which traps to kernel mode where `syscall()` dispatches to the handler.

## Key Conventions

### Header Files
- `kernel/types.h` - Basic types (uint64, int, etc.)
- `kernel/param.h` - System constants (NPROC, MAXARG, etc.)
- `kernel/defs.h` - Function prototypes across kernel files
- `kernel/riscv.h` - RISC-V specific definitions and inline assembly
- `user/user.h` - User-space syscall prototypes

### Function Naming
- Kernel functions implementing syscalls: `sys_name()`
- Actual kernel implementations: `kname()` (e.g., `kfork()`, `kexit()`)
- File system functions: Often prefix with `i` for inode ops (e.g., `ialloc()`, `ilock()`)

### Locking Patterns
- Spinlocks (`spinlock.h`): For short critical sections, disable interrupts
- Sleep locks (`sleeplock.h`): For longer operations that may sleep
- Always acquire locks in consistent order to avoid deadlock
- Most structures document which lock protects them

### Error Handling
- Syscalls return -1 on error (user-space)
- Kernel functions return 0 on success, -1 on failure
- `panic()` for unrecoverable kernel errors

### Memory
- Kernel uses physical addresses for most operations
- User addresses always go through page tables
- Use `copyin()`/`copyout()` to safely transfer data kernel↔user
- Never dereference user pointers directly in kernel

### RISC-V Specifics
- Code targets rv64gc (64-bit, general + compressed instructions)
- Syscall args in registers a0-a5
- Syscall number in a7
- Return value in a0
- See `kernel/riscv.h` for CSR access macros

## Toolchain

This repository uses `riscv64-linux-gnu-gcc` toolchain (Fedora setup). The Makefile auto-detects available RISC-V toolchains. Common prefixes:
- `riscv64-linux-gnu-` (Fedora/Ubuntu)
- `riscv64-unknown-elf-` (upstream)
- `riscv64-none-elf-` (ARM/embedded)

Override with: `make TOOLPREFIX=your-prefix-`

## Important Build Flags

- `-ffreestanding -nostdlib` - Freestanding environment (no libc)
- `-march=rv64gc` - RISC-V 64-bit with extensions
- `-mcmodel=medany` - Code model for kernel linking
- `-fno-builtin-*` - Disable GCC built-ins (we provide our own)
- `-Wall -Werror` - All warnings are errors

## References

- [xv6 RISC-V Book](https://pdos.csail.mit.edu/6.828/2023/xv6/book-riscv-rev3.pdf) - Authoritative documentation
- [MIT 6.1810 Course](https://pdos.csail.mit.edu/6.828/2023/) - Lectures and labs
- [RISC-V Instruction Set Manual](https://riscv.org/technical/specifications/)
