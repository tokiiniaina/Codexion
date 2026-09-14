*This project has been created as part of the 42 curriculum by tokinira.*

<p align="center"> <img src="assets/codexion.jpeg" alt="Codexion — Dining Philosophers" width="100%"> </p>


# Description

**Codexion** is a 42 School project written in C that simulates a group of "coders" competing for a limited set of shared **USB dongles** in order to compile code, using POSIX threads (`pthread`), mutexes, and condition variables.

The scenario, as defined by the subject:

- One or more coders sit around a shared, circular co-working table.
- Coders cycle endlessly through three states: **compile → debug → refactor → compile …**
- Compiling requires **two dongles at once** (one "left", one "right"). There are as many dongles as coders, arranged so that dongle `d` is shared between coder `d` and coder `d-1` (circularly).
- A dongle that is released enters a **cooldown** period before it can be taken again.
- If a coder does not start compiling again within `time_to_burnout` ms of their last compile start (or the start of the simulation), they **burn out** and the simulation stops.
- The simulation also stops successfully once every coder has compiled at least `number_of_compiles_required` times.

This is fundamentally a **resource-allocation / scheduling problem**: several threads (coders) compete for a small pool of shared, mutually-exclusive resources (dongles), and the program must arbitrate access fairly, under a real-time-ish deadline (`time_to_burnout`), while avoiding deadlock, starvation, and data races — hence the need for careful synchronization (mutexes, condition variables) and a scheduling policy (FIFO or EDF).

# Instructions

## Compilation

```sh
make          # builds the codexion binary
make clean    # removes the objs/ directory
make fclean   # clean + removes codexion and test binaries
make re       # fclean + all
make tests_run  # builds tests/test_queue.c and tests/test_time.c (not part of grading)
```

The `Makefile` compiles with `-Wall -Wextra -Werror -pthread`, using `cc`, and provides the required `$(NAME)`, `all`, `clean`, `fclean`, `re` rules.

> The `tests_run` target references `tests/test_queue.c` and `tests/test_time.c`. These files were **not provided** for this README and their content could not be verified.

## Execution

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coder threads, also the number of dongles |
| `time_to_burnout` | ms without starting a compile before a coder burns out |
| `time_to_compile` | ms spent compiling (holding two dongles) |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | Simulation succeeds once every coder reaches this compile count |
| `dongle_cooldown` | ms a dongle stays unavailable after being released |
| `scheduler` | `fifo` or `edf` — how dongles are granted when contested |

## Schedulers

- **fifo**: requests are granted strictly in arrival order.
- **edf** (Earliest Deadline First): the request with the earliest `deadline = last_compile_start + time_to_burnout` is granted first; ties are broken by arrival order.

## Examples

```sh
./codexion 5 800 200 200 200 3 200 fifo
./codexion 4 900 200 200 100 5 100 edf
./codexion 1 800 200 200 200 3 200 fifo   # see "Known limitations" for the single-coder case
```

## Invalid argument handling

- The program requires **exactly 8 arguments** (`argc == 9`); otherwise it prints `Error: invalid number of arguments`.
- Each numeric argument must be a non-negative, digit-only integer that does not overflow `INT_MAX`; anything else (negative sign, letters, empty string, overflow) is rejected with `Error: invalid arguments`.
- `scheduler` must be exactly `fifo` or `edf`.
- `number_of_coders` must be at least `1`.

# Resources

- POSIX Threads Programming (pthreads) — general reference on `pthread_create`, `pthread_join`
- `man pthread_mutex_init`, `man pthread_mutex_lock` — mutex documentation
- `man pthread_cond_init`, `man pthread_cond_wait`, `man pthread_cond_signal`, `man pthread_cond_broadcast` — condition variable documentation
- `man gettimeofday`, `man clock_gettime` — time-related documentation
- Binary heap / priority queue references (e.g. introductory algorithms textbooks covering heapify-up/heapify-down array-based heaps)
- FIFO and EDF (Earliest Deadline First) scheduling — classic real-time scheduling concepts
- Deadlock, Coffman's conditions, and general concurrency references (operating systems textbooks)

## AI usage

*(To be completed by the author with the actual, specific ways AI tools were used — this subsection was intentionally left as a template rather than invented, per the project's honesty requirement.)*

AI assistance was used during development for tasks such as:
- [ ] Understanding pthreads / mutexes / condition variables concepts
- [ ] Understanding FIFO vs EDF scheduling
- [ ] Debugging specific issues (describe which)
- [ ] Reviewing synchronization logic (describe which parts)
- [ ] Designing test scenarios
- [ ] Explaining compiler/runtime errors
- [ ] Generating this README from the source code (this document)

Please replace the checklist above with a short, honest, specific account of what was actually asked of AI tools and which files/functions were affected, in line with the 42 AI usage policy.

# Blocking cases handled

| # | Case | Mechanism observed in the code |
|---|---|---|
| 1 | **Deadlock** | `reserve_dongles()` always reserves the two dongles for a coder in a fixed ascending index order (`get_coder_dongles()` sorts `first < second`), and if the second dongle cannot be reserved, the first is immediately rolled back (`is_reserved = 0`, `is_available = 1`). This consistent lock/reservation ordering plus rollback avoids the classic "hold one, wait for the other" deadlock between two coders sharing dongles. |
| 2 | **Coffman's conditions** | Mutual exclusion is enforced per dongle (`t_dongle_data.mutex`). Hold-and-wait is avoided as above (no coder holds one dongle while blocked waiting for the other — reservation of the pair is all-or-nothing). Circular wait is avoided by the fixed ascending reservation order. Preemption is not applicable (dongles are not forcibly taken away). |
| 3 | **Race conditions** | Shared mutable state is protected by dedicated mutexes: `state_mutex` for coder fields (`last_compile_start`, `compile_count`, `is_finished`, `has_permission`) and simulation-wide flags (`stop_simulation`, `finished_coders`); `queue_mutex` for the priority queue and `request_counter`; `log_mutex` for `printf` calls; one mutex per dongle for its own fields. |
| 4 | **Starvation** | The scheduler drains the whole pending queue each cycle and attempts dispatch in strict priority order (FIFO arrival order, or EDF deadline order) via `drain_queue()` / `dispatch_pending()`, re-queuing only requests it could not satisfy. See "Known limitations" for a caveat on scheduler-thread blocking under contention. |
| 5 | **Fair arbitration** | Implemented via the custom binary-heap priority queue (`compare_requests()` in `queue/priority_queue_heap.c`), ordering requests by `arrival_order` (FIFO) or by `deadline` then `arrival_order` (EDF). |
| 6 | **FIFO scheduling** | `compare_requests()` compares `arrival_order` when `scheduler == SCHEDULER_FIFO`; `arrival_order` is assigned under `queue_mutex` in `enqueue_compile_request()`. |
| 7 | **EDF scheduling** | `compare_requests()` compares `deadline` (`last_compile_start + time_to_burnout`, computed in `enqueue_compile_request()`) when `scheduler == SCHEDULER_EDF`, with `arrival_order` as tie-breaker. |
| 8 | **Dongle cooldown** | `unlock_dongle()` sets `available_at = get_time_ms() + cooldown` on release; `try_reserve_dongle()` refuses to reserve a dongle while `current_time < available_at`. |
| 9 | **Burnout detection** | A dedicated `monitor_routine()` thread polls every coder's `last_compile_start` against `time_to_burnout` roughly every 1 ms (`usleep(1000)`), under `state_mutex`. |
| 10 | **Precise burnout logging** | The monitor logs immediately (within the same tick, ~1 ms polling granularity) once a burnout is detected, intended to satisfy the subject's "within 10 ms" requirement — actual precision depends on OS thread scheduling and was not independently timed for this README. |
| 11 | **Simulation termination** | `monitor_tick()` sets `stop_simulation = 1` (under `state_mutex`) either when `finished_coders == number_of_coders` or when a burnout is found, then calls `wake_everyone()`. |
| 12 | **Thread shutdown** | `wake_everyone()` signals every coder's `cond` and broadcasts `queue_cond`; every wait loop (`wait_for_permission`, `wait_for_queue`, the single-dongle busy loop in `take_dongles`) re-checks `stop_simulation` and exits. `start_simulation()` joins all coder threads, then the scheduler thread, then the monitor thread. |
| 13 | **Logging serialization** | `log_event()` wraps every `printf` call with `log_mutex` lock/unlock, so two log lines cannot interleave. |
| 14 | **Shared resource protection** | Each dongle has its own mutex guarding `is_available`, `is_reserved`, `available_at`; the mutex is held by the coder thread for the entire compile duration (locked in `take_dongles()`, unlocked in `release_dongles()` / `unlock_dongle()`). |

# Thread synchronization mechanisms

| Primitive | Where declared | Used for |
|---|---|---|
| `pthread_mutex_t mutex` (per dongle) | `t_dongle_data` | Guarding a single dongle's availability/reservation/cooldown state, and — because it is held for the full compile duration — also serializing actual physical access to that dongle |
| `pthread_mutex_t state_mutex` | `t_simulation_data` | Guarding coder fields (`last_compile_start`, `compile_count`, `is_finished`, `has_permission`) and simulation-wide flags (`stop_simulation`, `finished_coders`) |
| `pthread_mutex_t queue_mutex` + `pthread_cond_t queue_cond` | `t_simulation_data` | Guarding the priority queue and `request_counter`; the scheduler thread `pthread_cond_wait`s on `queue_cond` until a request is enqueued or the simulation stops |
| `pthread_cond_t cond` (per coder) | `t_coder_data` | The coder thread `pthread_cond_wait`s on it in `wait_for_permission()` until the scheduler grants permission (`has_permission = 1`) via `pthread_cond_signal()`, or the simulation stops |
| `pthread_mutex_t log_mutex` | `t_simulation_data` | Serializing all `printf` calls in `log_event()` |
| `pthread_create` / `pthread_join` | `simulation.c` | One thread per coder (`coder_routine`), plus one `scheduler_thread` and one `monitor_thread`; all are joined in `join_threads()` |

Per-area synchronization:

1. **Dongles** — protected individually (fine-grained locking, one mutex per dongle) rather than a single global dongle lock, allowing independent dongles to be manipulated concurrently.
2. **Scheduler / request queue** — `queue_mutex` + `queue_cond`: coders push a request under the lock and signal; the scheduler thread waits on the same condition variable, drains the queue, and releases the lock before attempting (potentially slow) dongle reservations.
3. **Coder state** — `state_mutex` is the single source of truth for `last_compile_start`, `compile_count`, `is_finished`, and `has_permission`, read/written by the coder itself, the scheduler (`grant_permission`), and the monitor.
4. **Monitor state** — the monitor takes `state_mutex` on every tick before inspecting coder data, so it never reads a torn/partial update.
5. **Logging** — a single `log_mutex` around every `log_event()` call from any thread (coders, scheduler, monitor).
6. **Simulation termination** — `stop_simulation` is only ever written under `state_mutex` and only ever read through `is_simulation_stopped()` (which itself locks `state_mutex`), so no thread observes a torn write of the flag; `wake_everyone()` then notifies both condition variables so blocked threads re-check it promptly.

Example — how a race is prevented: when a coder calls `enqueue_compile_request()`, it reads `last_compile_start` under `state_mutex`, then computes and pushes the request under `queue_mutex`. Because `request.arrival_order` is assigned from `simulation->request_counter` while `queue_mutex` is held, two coders enqueuing "simultaneously" can never receive the same `arrival_order`, keeping FIFO/EDF ordering deterministic.

# Architecture

| File | Responsibility |
|---|---|
| `main.c` | Argument-count check, calls `parse_arguments`, runs the simulation, prints final coder/dongle state |
| `parsing.c` | Validates and converts the 8 CLI arguments into `t_simulation_config` |
| `time.c` | `get_time_ms()` — millisecond timestamp via `gettimeofday` |
| `init.c` | Allocates and initializes all simulation state: mutexes/cond vars, coder/dongle/thread arrays, priority queue |
| `cleanup.c` | Destroys mutexes/cond vars and frees all heap allocations (`free_simulation`) |
| `states.c` | Thread-safe accessors: `is_simulation_stopped`, `mark_coder_finished`, `get_compile_count`, `enqueue_compile_request` |
| `simulation.c` | Spawns the monitor, scheduler, and coder threads; joins them all at the end (`start_simulation`) |
| `coder.c` | Per-coder thread loop (`coder_routine`): repeats the request → permission → compile → debug → refactor cycle |
| `coder_actions.c` | The individual coder-cycle steps: `wait_for_permission`, `take_dongles`, `compile_coder`, `finish_compile_cycle` |
| `dongle.c` | Dongle reservation/release logic, cooldown enforcement, mapping a coder id to its two dongle indices |
| `scheduler.c` | `scheduler_routine`: waits for pending requests, drains the queue, dispatches |
| `scheduler_dispatch.c` | `drain_queue`, `dispatch_pending`, `try_dispatch_request`, `grant_permission` |
| `monitor.c` | `monitor_routine`: polls for burnout / completion and stops the simulation |
| `logging.c` | `log_event`: timestamped, mutex-serialized log line |
| `queue/priority_queue.c` | Array-based binary heap: `init_priority_queue`, `push_request`, `pop_request` |
| `queue/priority_queue_heap.c` | `compare_requests` (FIFO/EDF ordering), `heapify_up`, `heapify_down` |

Key structures: `t_simulation_config` (parsed CLI arguments), `t_coder_data` (per-coder state + its own condition variable), `t_dongle_data` (per-dongle state + its own mutex), `t_compile_request` (coder id, deadline, arrival order — the unit stored in the queue), `t_priority_queue` (the heap array), `t_simulation_data` (everything above plus all shared mutexes/condition variables), `t_coder_context` (a `{coder, simulation}` pair passed to each coder thread).

Relationship: each **coder thread** pushes a `t_compile_request` into the shared **priority queue**, then waits on its own condition variable. The **scheduler thread** is the only consumer of the queue; it tries to reserve the requested pair of **dongles** and, on success, signals the coder. The **monitor thread** independently and continuously inspects coder state to detect burnout or completion and can stop the whole simulation at any time.

# Scheduling

The `scheduler` argument selects how the queue orders and grants pending compile requests:

- **FIFO**: request with the smallest `arrival_order` (the order in which coders called `enqueue_compile_request`) is served first.
- **EDF**: request with the smallest `deadline` (`last_compile_start + time_to_burnout`, i.e. the coder closest to burning out) is served first; ties fall back to `arrival_order`.

Both policies are implemented through a single custom **binary min-heap** (`queue/priority_queue*.c`), with `compare_requests()` swapping in the ordering rule based on `t_scheduler`.

Example: coder A last compiled at `t=0` with `time_to_burnout=1000`; coder B last compiled at `t=500` with the same burnout time. Under **FIFO**, whichever of A or B *requested* a compile first gets priority, regardless of urgency. Under **EDF**, A's deadline (`1000`) is earlier than B's (`1500`), so A is served first even if B requested slightly earlier — EDF favors the coder closer to burnout.

# Concurrency model

Each coder thread (`coder_routine` / `coder_cycle`) repeats:

```
request compile  →  wait for permission  →  acquire (reserve + take) both dongles
   →  compile  →  release dongles  →  debug  →  refactor  →  request again
```

1. **Request**: `enqueue_compile_request()` pushes a `t_compile_request` into the shared queue and signals the scheduler.
2. **Permission**: the coder blocks in `wait_for_permission()` on its own condition variable until the scheduler thread reserves its two dongles and calls `grant_permission()`.
3. **Acquire dongles**: `take_dongles()` locks both dongle mutexes (already flagged "reserved" by the scheduler) and logs `has taken a dongle` for each.
4. **Compile**: `compile_coder()` updates `last_compile_start`, logs `is compiling`, and sleeps `time_to_compile` ms while holding both dongle mutexes.
5. **Release**: `release_dongles()` unlocks both dongles, starting their cooldown (`available_at`).
6. **Debug / Refactor**: `finish_compile_cycle()` sleeps `time_to_debug` then `time_to_refactor` ms, logging each transition, then increments `compile_count` and checks completion.
7. Loop back to step 1, unless the simulation has stopped or the coder has reached `number_of_compiles_required`.

Meanwhile, the **scheduler thread** continuously tries to satisfy the highest-priority pending request(s), and the **monitor thread** continuously watches for burnout or overall completion, able to halt every coder at any point in this cycle via `stop_simulation`.

# Memory management

| Allocation | Where | Freed |
|---|---|---|
| `simulation->coders` (`t_coder_data[n]`) | `alloc_simulation_arrays` (`init.c`) | `free_simulation` (`cleanup.c`) |
| `simulation->threads` (`pthread_t[n]`) | `alloc_simulation_arrays` | `free_simulation` |
| `simulation->dongles` (`t_dongle_data[n]`) | `alloc_simulation_arrays` | `free_simulation` |
| `simulation->queue.requests` (heap array) | `init_priority_queue` | `free_simulation` |
| `contexts` (`t_coder_context[n]`) | `start_simulation` | freed locally at the end of `start_simulation` (both success and failure paths) |
| `pending` (per-scheduler-iteration buffer) | `scheduler_routine` | freed when the scheduler thread returns |

Initialization and cleanup are paired and ordered: `init_simulation()` sets up mutexes/condition variables first (`init_sync_primitives`), then allocates arrays, then initializes each coder's condition variable and each dongle's mutex — with every failure branch unwinding only what was already created (destroying the mutexes/cond vars that succeeded, freeing arrays already allocated) before returning an error, so a failed initialization does not call `free_simulation` on state it never touched (each `init.c` failure path performs its own partial cleanup instead). On a successful run, `free_simulation()` destroys the per-coder condition variables, the three simulation-wide mutexes, and every per-dongle mutex, then frees all four arrays.

No leaks were identified by inspection, but this was **not verified with a runtime tool** (e.g. Valgrind or `-fsanitize=address`) as part of this README — the author should confirm that independently.

# Error handling

| Case | Handling |
|---|---|
| Wrong argument count | `main.c` checks `argc != 9` before parsing |
| Invalid numeric argument (non-digit, negative, empty, overflow) | `parse_positive_number()` rejects it; `parse_arguments()` propagates the failure |
| Invalid scheduler string | `parse_scheduler()` accepts only `"fifo"` / `"edf"` |
| `number_of_coders < 1` | Explicitly rejected in `parse_arguments()` |
| `malloc` failure (coders/threads/dongles arrays, priority queue, contexts, scheduler's pending buffer) | Checked; on failure `init_simulation`/`start_simulation` unwind what was already allocated via `free_simulation()` and return an error code (the scheduler's own `pending` buffer failure simply returns from the thread without unwinding simulation state, since it is allocated after the simulation is already running) |
| `pthread_mutex_init` / `pthread_cond_init` failure | Checked in `init_sync_primitives`, `init_coders`, `init_dongles`; already-created primitives are destroyed before returning an error |
| `pthread_create` failure (monitor, scheduler, or any coder thread) | Checked in `spawn_all`; already-spawned threads are stopped (`stop_and_wake`) and joined before returning an error |

`main()` prints a short `Error: ...` message and returns `1` on any of the above failures; it does not attempt to continue with a partially valid configuration.

# Testing

The following are illustrative commands only — no results are reported here; run them yourself and verify the output/log format against the subject.

- **Basic execution**: `./codexion 3 1000 200 200 200 3 100 fifo`
- **One coder**: `./codexion 1 1000 200 200 200 3 100 fifo` (see limitation below)
- **Multiple coders**: `./codexion 6 1500 200 200 200 5 150 fifo`
- **FIFO**: `./codexion 4 1000 200 200 200 4 100 fifo`
- **EDF**: `./codexion 4 1000 200 200 200 4 100 edf`
- **Burnout**: set `time_to_burnout` lower than `time_to_compile + time_to_debug + time_to_refactor`, e.g. `./codexion 3 300 200 200 200 5 100 fifo`
- **Cooldown**: set a large `dongle_cooldown` relative to `time_to_compile` and observe delayed re-acquisition in the logs
- **Contention / stress**: increase `number_of_coders` significantly, e.g. `./codexion 30 2000 200 200 200 5 100 edf`
- **Deadlock check**: run with high contention under both schedulers and confirm the program always terminates (no permanent hang)
- **Starvation / fairness**: run EDF with many coders and check no single coder is repeatedly denied dongles until burnout, when parameters are feasible
- **Memory checking**: `valgrind --leak-check=full ./codexion ...`
- **Thread-synchronization checking**: `valgrind --tool=helgrind ./codexion ...` or `-fsanitize=thread` build
- **Timeout tests**: run with a `timeout` wrapper (e.g. `timeout 30 ./codexion ...`) to catch unexpected hangs

# Known limitations

The following were identified from reading the code and are **not** verified as failures at runtime — they are flagged for the author to check:

- **Single-coder case never compiles.** When `number_of_coders == 1`, `get_coder_dongles()` sets `second = -1`, and `take_dongles()`'s single-dongle branch only busy-waits (`usleep(1000)` in a loop) until the simulation stops, then releases the dongle — `compile_coder()` is never called. In this configuration the coder will never log `is compiling`/`is debugging`/`is refactoring`, `compile_count` never advances, and the coder will eventually burn out. Verify whether this is the intended behavior for `number_of_coders = 1`.
- **Scheduler thread can stall on a busy dongle.** A dongle's mutex is held by its current coder for the entire compile duration (`take_dongles()` → `release_dongles()`). Since `try_reserve_dongle()` must lock that same mutex to evaluate a pending request for that dongle, the single scheduler thread can block on `pthread_mutex_lock` for up to the remaining compile time while checking one request, delaying evaluation of other pending requests (potentially for unrelated, free dongles) in the same dispatch cycle. This could affect the subject's liveness/fairness expectations under contention and is worth testing under load, especially with EDF.
- **`last_compile_start` is set twice per cycle** — once in `grant_permission()` (scheduler, at dispatch time) and again in `compile_coder()` (coder, right before actually compiling). The second write wins and is the more accurate one; the first is redundant but not incorrect.
- **Monitor and scheduler are polling-based** (`usleep(1000)` loops) rather than purely event-driven for burnout detection and re-dispatch after a failed reservation, which adds a small (~1 ms scale) timing overhead.
- `pthread_cond_timedwait` and `clock_gettime`, though listed as permitted external functions in the subject, are not used anywhere in the reviewed code (`pthread_cond_wait` and `gettimeofday` are used instead) — not a violation, just unused allowances.
- `tests/test_queue.c` and `tests/test_time.c`, referenced by the `tests_run` Makefile target, were not provided and could not be reviewed.
- Norm (42 coding style) compliance was not checked as part of this README.

# Project structure

```
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
    ├── test_queue.c   (referenced by Makefile, not reviewed)
    └── test_time.c    (referenced by Makefile, not reviewed)
```

# Evaluation / Defense

Concepts to be able to explain, with where they live in this codebase:

| Concept | Where in the project |
|---|---|
| pthreads | `pthread_create`/`pthread_join` for coders, scheduler, monitor (`simulation.c`) |
| Mutexes | Per-dongle mutex, `state_mutex`, `queue_mutex`, `log_mutex` (`codexion.h`, used throughout) |
| Condition variables | Per-coder `cond` (`coder_actions.c`), `queue_cond` (`scheduler.c`, `states.c`) |
| Race conditions | Shared state guarded by `state_mutex` / `queue_mutex` (`states.c`) |
| Deadlocks | Fixed ascending dongle reservation order + rollback (`dongle.c`) |
| Coffman's conditions | Discussed above in "Blocking cases handled" |
| Starvation | FIFO/EDF priority ordering in the heap (`queue/`) |
| FIFO | `compare_requests()` on `arrival_order` |
| EDF | `compare_requests()` on `deadline = last_compile_start + time_to_burnout` |
| Priority queues / heaps | Custom array-based binary heap (`queue/priority_queue*.c`) |
| Dongle cooldown | `available_at` field, checked in `try_reserve_dongle()` |
| Burnout | `find_burned_out()` in `monitor.c` |
| Monitor thread | `monitor_routine()` |
| Logging synchronization | `log_mutex` in `log_event()` (`logging.c`) |

Also be ready to discuss the two limitations noted above (single-coder behavior, scheduler-thread stall on a busy dongle), since they are the most likely follow-up questions during a defense.