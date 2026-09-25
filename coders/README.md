*This project has been created as part of the 42 curriculum by tokinira.*

<p align="center">
  <img src="assets/codexion_upgraded.png" alt="Codexion — Dining Philosophers" width="100%">
</p>

# Description

**Codexion** is a 42 School project written in C. It simulates multiple coders competing for shared USB dongles using POSIX threads, mutexes, condition variables and a custom scheduler.

Each coder repeatedly follows:

**compile → debug → refactor → compile**

Compiling requires two neighboring dongles simultaneously. Released dongles remain unavailable during their cooldown period.

The simulation stops when:

* a coder burns out because they did not start compiling before their deadline;
* or every coder has completed the required number of compilations.

The project implements two scheduling policies: **FIFO** and **EDF**.

# Instructions

## Compilation

```sh
make
make clean
make fclean
make re
```

The project is compiled with:

```text
-Wall -Wextra -Werror -pthread
```

## Execution

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                      | Description                           |
| ----------------------------- | ------------------------------------- |
| `number_of_coders`            | Number of coders and dongles          |
| `time_to_burnout`             | Maximum time before a coder burns out |
| `time_to_compile`             | Compilation duration                  |
| `time_to_debug`               | Debugging duration                    |
| `time_to_refactor`            | Refactoring duration                  |
| `number_of_compiles_required` | Compilations required per coder       |
| `dongle_cooldown`             | Dongle cooldown duration              |
| `scheduler`                   | `fifo` or `edf`                       |

Example:

```sh
./codexion 5 800 200 200 200 3 200 fifo
./codexion 4 900 200 200 100 5 100 edf
```

## Argument validation

The program rejects:

* an incorrect number of arguments;
* invalid or overflowing numeric values;
* negative values;
* invalid scheduler names;
* fewer than one coder.

# Scheduling

### FIFO

Requests are processed according to their arrival order.

### EDF

**Earliest Deadline First** prioritizes the request with the earliest burnout deadline:

```text
deadline = last_compile_start + time_to_burnout
```

When deadlines are equal, arrival order is used as the tie-breaker.

Both policies use the project's custom binary heap priority queue.

# Resources

* POSIX Threads: `pthread_create`, `pthread_join`, mutexes and condition variables
* `man pthread_mutex_*`
* `man pthread_cond_*`
* `man gettimeofday`
* [`pthread_cond_wait(3p)` — POSIX.1-2017 / Open Group Base Specifications](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html)
* [`pthread_cond_wait(3)` — Linux man-pages](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html)
* Binary heaps and priority queues
* FIFO and EDF scheduling
* Operating systems and concurrency references
* Deadlocks and Coffman's conditions

## AI usage

*(To be completed by the author with the actual, specific ways AI tools were used — this subsection was intentionally left as a template rather than invented, per the project's honesty requirement.)*

AI assistance was used during development for tasks such as:

* [ ] Understanding pthreads / mutexes / condition variables concepts
* [ ] Understanding FIFO vs EDF scheduling
* [ ] Debugging specific issues (describe which)
* [ ] Reviewing synchronization logic (describe which parts)
* [ ] Designing test scenarios
* [ ] Explaining compiler/runtime errors
* [ ] Generating this README from the source code (this document)

# Blocking cases handled

| Case                     | Mechanism                                                                                                   |
| ------------------------ | ----------------------------------------------------------------------------------------------------------- |
| **Deadlock**             | Dongles are reserved in a consistent order and reservations are rolled back if the pair cannot be obtained. |
| **Coffman's conditions** | Per-dongle mutexes provide mutual exclusion; fixed ordering prevents circular wait.                         |
| **Race conditions**      | Shared simulation state, queue state, logging and dongle state are protected by dedicated mutexes.          |
| **Starvation**           | Requests are ordered through FIFO or EDF priority policies.                                                 |
| **Fair arbitration**     | A custom binary heap determines which pending request is considered first.                                  |
| **Dongle cooldown**      | `available_at` prevents a dongle from being reused before its cooldown expires.                             |
| **Burnout**              | A dedicated monitor checks coder deadlines independently from coder threads.                                |
| **Log serialization**    | `log_mutex` protects every output operation.                                                                |
| **Termination**          | The simulation stops on burnout or when all coders reach the required compile count.                        |

# Thread synchronization mechanisms

| Primitive                         | Purpose                                                                  |
| --------------------------------- | ------------------------------------------------------------------------ |
| `pthread_mutex_t` per dongle      | Protects dongle availability, reservation and cooldown state.            |
| `state_mutex`                     | Protects coder state and simulation termination state.                   |
| `queue_mutex`                     | Protects the priority queue and request counter.                        |
| `pthread_cond_t cond`             | Allows each coder to wait for scheduler permission.                      |
| `pthread_cond_t queue_cond`       | Wakes the scheduler when requests are available or the simulation stops. |
| `log_mutex`                       | Serializes log output.                                                   |
| `pthread_create` / `pthread_join` | Creates and synchronizes coder, scheduler and monitor threads.           |

The main synchronization flow is:

```text
Coder
  │
  ├── enqueue request ──→ Priority Queue
  │                           │
  │                           ▼
  │                       Scheduler
  │                           │
  │                     reserve dongles
  │                           │
  │                           ▼
  └──── wait for permission ←─┘
              │
              ▼
          Compile
              │
              ▼
      Release + cooldown
              │
              ▼
       Debug → Refactor
```

The monitor independently checks coder state and can stop the simulation by updating the shared termination state and waking waiting threads.

### Waiting strategy

We use `pthread_cond_wait` wherever a thread can be woken by a specific, predictable event (a granted permission, a new request in the queue), since it lets the thread sleep with zero CPU usage until explicitly signaled. The only exception is the degenerate case of a single coder (`N=1`), where the coder can never acquire a second dongle and therefore has no event to wait for; there, a lightweight polling loop (`usleep(1000)`) is used instead, since its CPU cost is negligible for this one edge case and avoids adding a dedicated condition variable for a scenario that never occurs in a real multi-coder simulation.

### Why `pthread_cond_wait` is always called inside a loop

Per the POSIX specification, `pthread_cond_wait` may return even when no thread has signaled the condition variable — these are called **spurious wakeups** — and a signaled thread is never guaranteed to be the one that re-checks the shared state first. For this reason, the predicate must always be re-evaluated in a loop after waking up:

```c
pthread_mutex_lock(&mutex);
while (!condition)
    pthread_cond_wait(&cond, &mutex);
/* condition is now guaranteed true */
pthread_mutex_unlock(&mutex);
```

References:
* [`pthread_cond_wait(3p)` — POSIX.1-2017 / Open Group Base Specifications](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html)
* [`pthread_cond_wait(3)` — Linux man-pages](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html)

# Architecture

| File                   | Responsibility                            |
| ---------------------- | ------------------------------------------ |
| `main.c`               | Program entry point and argument handling |
| `parsing.c`            | Argument validation and configuration     |
| `time.c`               | Millisecond timestamps                    |
| `init.c`               | Simulation initialization                 |
| `cleanup.c`            | Resource destruction and memory cleanup   |
| `states.c`             | Shared state and request management       |
| `simulation.c`         | Thread creation and joining               |
| `coder.c`              | Coder thread lifecycle                    |
| `coder_actions.c`      | Coder actions                             |
| `dongle.c`             | Dongle reservation and cooldown           |
| `scheduler.c`          | Scheduler thread                          |
| `scheduler_dispatch.c` | Request dispatching                       |
| `monitor.c`            | Burnout and completion monitoring         |
| `logging.c`            | Synchronized logging                      |
| `queue/`               | Binary heap and FIFO/EDF ordering         |

# Concurrency model

Each coder follows:

```text
Request
   ↓
Wait for permission
   ↓
Acquire two dongles
   ↓
Compile
   ↓
Release dongles
   ↓
Debug
   ↓
Refactor
   ↓
Request again
```

The scheduler manages pending compile requests, while the monitor independently watches burnout deadlines and simulation completion.

# Memory management

All dynamically allocated simulation resources are released during cleanup:

* coder data;
* thread handles;
* dongles;
* priority queue;
* temporary scheduler/context allocations.

Mutexes and condition variables are destroyed before their associated memory is freed.

# Error handling

Initialization and thread creation failures are checked and propagated.

Already-created synchronization primitives and allocated resources are cleaned up when an initialization step fails.

Invalid command-line arguments are rejected before the simulation starts.

# Testing

The following are illustrative commands only — no results are reported here; run them yourself and verify the output/log format against the subject.

* **Basic execution**: `./codexion 3 1000 200 200 200 3 100 fifo`
* **One coder**: `./codexion 1 1000 200 200 200 3 100 fifo` (see limitation below)
* **Multiple coders**: `./codexion 6 1500 200 200 200 5 150 fifo`
* **FIFO**: `./codexion 4 1000 200 200 200 4 100 fifo`
* **EDF**: `./codexion 4 1000 200 200 200 4 100 edf`
* **Burnout**: set `time_to_burnout` lower than `time_to_compile + time_to_debug + time_to_refactor`, e.g. `./codexion 3 300 200 200 200 5 100 fifo`
* **Cooldown**: set a large `dongle_cooldown` relative to `time_to_compile` and observe delayed re-acquisition in the logs
* **Contention / stress**: increase `number_of_coders` significantly, e.g. `./codexion 30 2000 200 200 200 5 100 edf`
* **Deadlock check**: run with high contention under both schedulers and confirm the program always terminates (no permanent hang)
* **Starvation / fairness**: run EDF with many coders and check no single coder is repeatedly denied dongles until burnout, when parameters are feasible
* **Memory checking**: `valgrind --leak-check=full ./codexion ...`
* **Thread-synchronization checking**: `valgrind --tool=helgrind ./codexion ...` or `-fsanitize=thread` build
* **Timeout tests**: run with a `timeout` wrapper (e.g. `timeout 30 ./codexion ...`) to catch unexpected hangs

# Known limitations

The following were identified from reading the code and are **not** verified as failures at runtime — they are flagged for the author to check:

* **Single-coder case never compiles.** When `number_of_coders == 1`, `get_coder_dongles()` sets `second = -1`, and `take_dongles()`'s single-dongle branch only busy-waits (`usleep(1000)` in a loop) until the simulation stops, then releases the dongle — `compile_coder()` is never called. In this configuration the coder will never log `is compiling`/`is debugging`/`is refactoring`, `compile_count` never advances, and the coder will eventually burn out. Verify whether this is the intended behavior for `number_of_coders = 1`.
* **Scheduler thread can stall on a busy dongle.** A dongle's mutex is held by its current coder for the entire compile duration (`take_dongles()` → `release_dongles()`). Since `try_reserve_dongle()` must lock that same mutex to evaluate a pending request for that dongle, the single scheduler thread can block on `pthread_mutex_lock` for up to the remaining compile time while checking one request, delaying evaluation of other pending requests (potentially for unrelated, free dongles) in the same dispatch cycle. This could affect the subject's liveness/fairness expectations under contention and is worth testing under load, especially with EDF.
* **`last_compile_start`**** is set twice per cycle** — once in `grant_permission()` (scheduler, at dispatch time) and again in `compile_coder()` (coder, right before actually compiling). The second write wins and is the more accurate one; the first is redundant but not incorrect.
* **Monitor and scheduler are polling-based** (`usleep(1000)` loops) rather than purely event-driven for burnout detection and re-dispatch after a failed reservation, which adds a small (~1 ms scale) timing overhead.
* `pthread_cond_timedwait` and `clock_gettime`, though listed as permitted external functions in the subject, are not used anywhere in the reviewed code (`pthread_cond_wait` and `gettimeofday` are used instead) — not a violation, just unused allowances.
* `tests/test_queue.c` and `tests/test_time.c`, referenced by the `tests_run` Makefile target, were not provided and could not be reviewed.
* Norm (42 coding style) compliance was not checked as part of this README.

# Project structure

```text
codexion/
├── Makefile
├── includes/
│   └── codexion.h
├── srcs/
│   ├── main.c
│   ├── parsing.c
│   ├── time.c
│   ├── init.c
│   ├── cleanup.c
│   ├── states.c
│   ├── simulation.c
│   ├── coder.c
│   ├── coder_actions.c
│   ├── dongle.c
│   ├── scheduler.c
│   ├── scheduler_dispatch.c
│   ├── monitor.c
│   ├── logging.c
│   └── queue/
│       ├── priority_queue.c
│       └── priority_queue_heap.c
└── tests/
    ├── test_queue.c
    └── test_time.c
```