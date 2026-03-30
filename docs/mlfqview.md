# MLFQ Scheduler & mlfqview

Documentation for the Multi-Level Feedback Queue (MLFQ) scheduler changes and the `mlfqview` user utility.

---

## 1. Overview

This change implements a simple 4-level MLFQ scheduler (levels 3 → 0, where 3 is the highest priority). Processes are assigned to a priority level and get time slices based on that level. The kernel tracks per-process ticks and waiting time to drive demotion, promotion (priority boosting), and debugging.

The `getpinfo` syscall and the `mlfqview` user program provide a read-only view of per-process MLFQ state for debugging and observation.

## 2. Key concepts

- Priorities: integer levels 0..3 (3 highest, 0 lowest). New `struct proc` fields were added to track MLFQ state: `priority`, `ticks[4]`, `wait_ticks[4]`, and `curr_ticks`.
- Time slices: the helper `time_slice(int priority)` maps priorities to slices:

```c
if (priority == 3) return 8;
if (priority == 2) return 16;
if (priority == 1) return 32;
return -1; // level 0: effectively infinite (no automatic demotion)
```

- Demotion: on every timer interrupt the running process has `curr_ticks++` and `ticks[priority]++`. If `curr_ticks` reaches the configured slice for that priority (and the slice is finite), the process is demoted (priority--), `curr_ticks` is reset, and the process is preempted via `yield()`.
- Priority boosting (anti-starvation): the scheduler periodically increments `wait_ticks` for RUNNABLE processes. If a process has waited at its current level for >= `10 * time_slice(priority)` ticks, it is promoted (priority++), its `wait_ticks` for the new level is reset, and `curr_ticks` is cleared.
- Scheduling order: the `scheduler()` scans priority levels from 3 down to 0 and dispatches the first RUNNABLE process at the highest non-empty level (simple per-level FCFS/RR behavior).

## 3. `getpinfo` syscall and `mlfqview` utility

- `getpinfo(struct pinfo *info)` — kernel syscall that fills an array of `struct pinfo` with current MLFQ state for every active process and returns the number of entries written (or -1 on error).

- `struct pinfo` (kernel/user visible):

```c
struct pinfo {
  int pid;         // process id
  int priority;    // current priority (0..3)
  int curr_ticks;  // ticks used in current slice
  int ticks[4];    // total ticks accumulated at each priority
};
```

- `mlfqview` is a small user program that calls `getpinfo()` and prints a table of PID, priority, current-slice ticks, and the per-level tick totals.

Example output:

```
PID  PRIORITY  CURR_TICKS  TICKS[0-3]
3     2        4     0 12 4 0
4     3        1     0 0 1 3
```

## 4. Implementation notes & behavior

- Timer bookkeeping lives in `usertrap()`/`trap.c`: on timer interrupts the running process updates `curr_ticks` and `ticks[]` and then checks whether it exceeded its slice and should be demoted and preempted.
- The `scheduler()` (in `kernel/proc.c`) scans priorities top-down and runs the first runnable process at a given level. After scanning, it updates `wait_ticks` for runnable processes to implement boosting.
- Level 0 has an infinite slice (no automatic demotion). Processes at lower levels receive increasingly larger slices (higher numeric slice values for lower priority numbers in this implementation, except that level 0 is infinite).

## 5. Files changed / added

| File | Change |
|---|---|
| `kernel/proc.h` | Added MLFQ fields to `struct proc` (`priority`, `ticks[]`, `wait_ticks[]`, `curr_ticks`) and `struct pinfo` definition |
| `kernel/proc.c` | Initialize MLFQ fields in `allocproc()`, added `time_slice()` helper, scheduler scan that honors priorities, `getnproc`/`getpinfo` helpers and logic |
| `kernel/trap.c` | Timer interrupt updates `curr_ticks`/`ticks[]` and enforces demotion + preemption |
| `kernel/syscall.h` | Added `SYS_getpinfo` syscall number |
| `kernel/syscall.c` | Added `sys_getpinfo` declaration and dispatch table entry |
| `kernel/sysproc.c` | (If present) syscall wrapper `sys_getpinfo()` that collates and returns process info |
| `user/user.h` | Added `struct pinfo` forward decl and `int getpinfo(struct pinfo *info);` prototype |
| `user/user.c` | Added `getpinfo` syscall wrapper (calls `syscall(SYS_getpinfo, ...)`) |
| `user/mlfqview.c` | New user utility to call `getpinfo()` and display results |
| `Makefile` | Added `$U/_mlfqview` target to build the user program |

## 6. Debugging tips

- Run `mlfqview` from the shell while other jobs are running to observe dynamic priority changes and tick counters.
- Use `getpinfo()` from a small test program to capture snapshots and assert invariants (e.g., total ticks incrementing over time, promotions after long waits).

## 7. TODO / future enhancements

- Expose per-process wait_ticks for richer debugging output.
- Make time-slice and boost thresholds tunable via sysctl or kernel config.
- Track per-queue run queues to make scheduling O(1) instead of scanning the whole proc table.

### Screenshot
<!-- Add screenshot here -->
![alt text](mlfqview.png)
