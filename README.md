# RTOS Simulator

A Real-Time Operating System (RTOS) simulator written in C, demonstrating core RTOS concepts through a fully functional multi-file project.

## Features

| Feature | Description |
|---|---|
| **Priority Scheduling** | 9 priority levels (0–8), highest priority task runs first |
| **Preemptive Multitasking** | Timer-driven time slice expiration forces context switches |
| **POSIX Timer** | Uses `timer_create` and `SIGALRM` to drive the system tick engine |
| **Mutex** | Mutual exclusion lock with priority inheritance to prevent priority inversion |
| **Counting Semaphore** | Semaphore with configurable max count and blocked-task wake-up |
| **Task Queue** | Linked-list based queue with highest-priority dequeue support |
| **Simulated Context Switch** | Saves and restores simulated CPU register state with detailed logging |

## Project Structure

```
RTOS/
├── Makefile                  # Build system
├── README.md
├── include/
│   ├── rtos.h                # Master header — types, constants, includes
│   ├── task.h                # Task lifecycle API
│   ├── scheduler.h           # Priority preemptive scheduler API
│   ├── mutex.h               # Mutex with priority inheritance
│   ├── semaphore.h           # Counting semaphore API
│   ├── task_queue.h          # Priority-aware task queue API
│   ├── timer.h               # POSIX timer API
│   └── context.h             # Simulated CPU context switch API
├── src/
│   ├── task.c                # TCB pool, create/delete/suspend/resume
│   ├── scheduler.c           # Multi-level priority queues, preemption
│   ├── mutex.c               # Lock/unlock with priority inheritance
│   ├── semaphore.c           # Wait/post with waiter wake-up
│   ├── task_queue.c          # Enqueue/dequeue/highest-priority extract
│   ├── timer.c               # POSIX timer_create + SIGALRM handler
│   ├── context.c             # Simulated register save/restore/switch
│   └── main.c                # 8-phase demo exercising all features
└── build/                    # Compiled object files and binary
```

## Prerequisites

- **GCC** (MinGW-w64 on Windows, or native GCC on Linux/macOS)
- **Make** (GNU Make)
- **POSIX threads** (`-lpthread`)
- **POSIX realtime** (`-lrt`)

### Installing on MSYS2 (Windows)

```bash
pacman -S mingw-w64-x86_64-gcc make
```

### Installing on Ubuntu/Debian

```bash
sudo apt install gcc make
```

## Building

```bash
# Build the project
make

# Build and run
make run

# Clean build artifacts
make clean

# Full rebuild
make rebuild
```

## How It Works

The simulator runs through 8 phases when executed:

### Phase 1 — Initialization
Initializes the task pool, scheduler, mutex, and semaphore subsystems.

### Phase 2 — Task Creation
Creates 5 demo tasks at different priority levels:

| Task | Priority | Role |
|---|---|---|
| `HighPriTask` | 7 | Acquires mutex, modifies shared resource |
| `Periodic` | 6 | Heartbeat task, posts to semaphore |
| `MedPriTask` | 5 | Waits on semaphore, modifies shared resource |
| `QueueDemo` | 3 | Demonstrates task queue operations |
| `LowPriTask` | 2 | Acquires mutex, triggers priority inheritance |

### Phase 3 — Ready Queue Snapshot
Displays the initial state of all priority-level ready queues.

### Phase 4 — Timer Setup
Configures the POSIX timer to fire `SIGALRM` every 100ms (configurable via `TIME_SLICE_MS`).

### Phase 5 — Scheduler Execution
The scheduler dispatches tasks in priority order. Higher priority tasks preempt lower ones. The timer drives automatic preemption when a time slice expires.

### Phase 6 — Suspend / Resume
Demonstrates dynamic task suspension and resumption.

### Phase 7 — Context Inspection
Prints the simulated CPU context (registers, stack pointer, program counter, flags) for each task.

### Phase 8 — Cleanup
Stops the scheduler and timer, destroys mutex and semaphore resources.

## Configuration

Key constants can be adjusted in `include/rtos.h`:

```c
#define MAX_TASKS       16      // Maximum concurrent tasks
#define MAX_PRIORITY    8       // Highest priority level (0 = lowest)
#define STACK_SIZE      4096    // Per-task stack size in bytes
#define TIME_SLICE_MS   100     // Scheduler tick interval in milliseconds
#define TASK_NAME_LEN   32      // Max task name length
```

## Key Concepts Demonstrated

### Priority Inheritance
When a high-priority task blocks on a mutex held by a low-priority task, the low-priority task's priority is temporarily boosted to prevent **priority inversion**.

### Preemptive Scheduling
The POSIX timer generates periodic `SIGALRM` signals. When a task's time slice expires, the scheduler preempts it and dispatches the next highest-priority ready task.

### Context Switching
Each task has a simulated `cpu_context_t` containing 16 general-purpose registers, a stack pointer, program counter, and flags register. On every switch, the outgoing task's context is saved and the incoming task's context is restored.

## License

This project is under [MIT LICENSE](https://github.com/BipronathSaha12/RTOS/blob/main/LICENSE) 
