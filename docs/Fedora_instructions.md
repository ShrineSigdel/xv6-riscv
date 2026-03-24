# xv6-riscv

A personal fork of [MIT's xv6-riscv](https://github.com/mit-pdos/xv6-riscv) — a small, Unix-like teaching operating system built for the RISC-V architecture. The goal of this repo is to explore OS internals by modifying the kernel and implementing custom system calls for educational purposes.

---

## Environment

| | |
|---|---|
| **OS** | Fedora |
| **Toolchain** | riscv64-linux-gnu-gcc |
| **Emulator** | qemu-system-riscv64 |

---

## Setup

### 1. Install virtualization support

QEMU requires Fedora's virtualization group to be installed first.

```bash
sudo dnf install -y @virtualization
```

### 2. Install QEMU for RISC-V

Install the RISC-V system emulator to run xv6.

```bash
sudo dnf install -y qemu-system-riscv
```

### 3. Install build dependencies

Tools needed to compile the xv6 kernel and userspace programs.

```bash
sudo dnf install -y git make gcc gcc-c++ \
    gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu \
    autoconf automake python3 bison flex texinfo \
    libmpc-devel mpfr-devel gmp-devel \
    zlib-devel expat-devel libslirp-devel ncurses-devel
```

### 4. Clone the repository

```bash
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
```

---

## Build & Run

```bash
make qemu
```

You should see:

```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$
```

To exit xv6: press `Ctrl+A` then `X`.

---

## Testing

Some useful commands to try inside the xv6 shell:

```sh
ls                          # list files in the filesystem
cat README                  # read the README
echo "hello" > file.txt     # create a file
cat file.txt                # read it back
rm file.txt                 # delete it
grep the README             # search text in a file
wc README                   # word count
mkdir mydir                 # create a directory
ls | grep README            # pipe commands
forktest                    # test fork() system call
stressfs                    # stress test the filesystem
usertests                   # run the full xv6 test suite
```

---

## Status

- [x] Successfully compiled
- [x] Boots in QEMU
- [x] Shell responsive
- [x] All built-in tests passing

---

## Branch Note
The original xv6 repository from MIT PDOS uses the riscv branch as its primary branch.
So, use :

```bash
git push origin riscv
```

## References

- [xv6-riscv source (MIT)](https://github.com/mit-pdos/xv6-riscv)
- [xv6 book (PDF)](https://pdos.csail.mit.edu/6.828/2023/xv6/book-riscv-rev3.pdf)
- [MIT 6.1810 OS course](https://pdos.csail.mit.edu/6.828/2023/)