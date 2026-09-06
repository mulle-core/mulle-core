# mulle-multififo Library Documentation for AI
<!-- Keywords: lock-free, fifo, threads, queue, mpmc, concurrency -->

## 1. Introduction & Purpose

mulle-multififo provides multi-producer/multi-consumer (MPMC) FIFO queues
holding `void *` pointers, written for multi-thread configurations. An
arbitrary number of threads may push and pop pointers concurrently. The queue
size is set at runtime (during initialization) and both a lock-free and a
mutex-locking variant are offered. Operations are non-blocking: a write to a
full queue returns -1 immediately, and a read from an empty queue returns
NULL immediately.

It is the multi-threaded counterpart of
[mulle-fifo](//github.com/mulle-concurrent/mulle-fifo), which supports only a
single producer/single consumer pair. This library is a component of the
`mulle-core` / `mulle-concurrent` ecosystem and is typically pulled in through
those aggregates rather than added individually.

## 2. Key Concepts & Design Philosophy

- **Multiple producers and consumers:** Both variants allow many threads to
  read and write at the same time.
- **Two variants with the same call pattern:**
  - `mulle_pointermultififo` — lock-free, built on atomic pointer operations
    (CAS) from `mulle-thread`.
  - `mulle_lockingpointermultififo` — simpler, serialized by a
    `mulle_thread_mutex`.
- **Non-blocking protocol:** `_write` returns 0 on success and -1 (with
  `errno` set) when the queue is full; `_read_barrier` returns the pointer, or
  NULL when the queue is empty. No call ever waits for another thread.
- **Sentinel values:** NULL and `(void *) ~0` cannot be stored. NULL is the
  "empty slot" marker and `(void *) ~0` is the `NOTYET` in-flight marker used
  by the lock-free algorithm.
- **Cyclic ring buffer:** Both `read` and `write` are monotonically
  increasing counters, indexed into preallocated storage as
  `counter % size`. Slots are reclaimed implicitly by index wrap-around; the
  "full" condition is detected on the lock-free side through the `NOTYET`
  sentinel and on the locking side through an explicit element counter `n`.
- **Memory barriers:** The `_read_barrier` variants guarantee that the memory
  contents pointed to by the returned pointer are consistent for the reading
  thread (the locking variant gets this implicitly from its mutex).

## 3. Core API & Data Structures

All public API lives in `mulle-multififo/mulle-multififo.h` which includes
the two "pointermultififo" headers. All signatures below are copied verbatim.

### 3.1. `mulle-multififo.h`

Version accessors. The encoded version is
`MULLE__MULTIFIFO_VERSION` `((1UL << 20) | (0 << 8) | 5)` (i.e. 1.0.5).

```c
static inline unsigned int   mulle_multififo_get_version_major( void)
```

```c
static inline unsigned int   mulle_multififo_get_version_minor( void)
```

```c
static inline unsigned int   mulle_multififo_get_version_patch( void)
```

```c
MULLE__MULTIFIFO_GLOBAL
uint32_t   mulle_multififo_get_version( void)
```

### 3.2. `mulle-pointermultififo.h` (lock-free variant)

#### `struct mulle_pointermultififo`

```c
struct mulle_pointermultififo
{
   mulle_atomic_pointer_t   read;     // only accessed by consumers
   mulle_atomic_pointer_t   write;    // only accessed by producers
   unsigned int             size;     // read only after init
   struct mulle_allocator   *allocator;
   mulle_atomic_pointer_t   *storage;
};
```

- **Purpose:** Lock-free MPMC FIFO queue of `void *`. The queue has no
  internal locking; thread-safety comes from atomic pointer operations.
- **Key Fields:**
  - `read` / `write`: Atomic circular indices; consumers CAS `read`,
    producers CAS `write`.
  - `size`: Capacity set at init; must be at least 2.
  - `allocator`: Allocator used to obtain `storage` (may be NULL for the
    default allocator).
  - `storage`: Preallocated, zeroed ring buffer of `mulle_atomic_pointer_t`.
- **Lifecycle Functions:**
  - `_init`: `void _mulle_pointermultififo_init( struct mulle_pointermultififo *p, unsigned int size, struct mulle_allocator *allocator)` — zeroes the struct, records `size` (asserts `size >= 2`) and `allocator`, then `mulle_allocator_calloc`s the storage.
  - `_done`: `static inline void _mulle_pointermultififo_done( struct mulle_pointermultififo *p)` — frees `p->storage` with `mulle_allocator_free`. Must only run when no other thread uses the FIFO.
- **Core Operations:**
  - `_read_barrier`: `void *_mulle_pointermultififo_read_barrier( struct mulle_pointermultififo *p)` — pops the next element, or returns NULL if empty (never blocks). Uses a CAS-based protocol with the `NOTYET` sentinel so concurrent readers do not double-pop. Ends with an atomic memory barrier so the memory behind the returned pointer is valid.
  - `_write`: `int _mulle_pointermultififo_write( struct mulle_pointermultififo *p, void *pointer)` — pushes an element, or fails (non-blocking). Returns 0 on success; -1 with `errno == EBUSY` if full; -1 with `errno == EINVAL` if `pointer` is NULL or `(void *) ~0`.

### 3.3. `mulle-lockingpointermultififo.h` (mutex variant)

#### `struct mulle_lockingpointermultififo`

```c
struct mulle_lockingpointermultififo
{
   mulle_thread_mutex_t     lock;
   unsigned int             n;
   unsigned int             write;    // only accessed by producers
   unsigned int             read;     // only accessed by consumers
   unsigned int             size;     // read only after init
   struct mulle_allocator   *allocator;
   void                     **storage;
};
```

- **Purpose:** MPMC FIFO queue of `void *` protected by a
  `mulle_thread_mutex_t`. Simpler than the lock-free variant and additionally
  tracks the current number of queued elements.
- **Key Fields:**
  - `lock`: Mutex guarding `n`, `read`, `write`, `storage`.
  - `n`: Current number of queued elements (drives full/empty checks).
  - `read` / `write`: Circular indices (plain `unsigned int`).
  - `size`: Capacity set at init; must be at least 2.
  - `allocator`: Allocator used to obtain `storage`.
  - `storage`: Preallocated ring buffer of `void *`.
- **Lifecycle Functions:**
  - `_init`: `void _mulle_lockingpointermultififo_init( struct mulle_lockingpointermultififo *p, unsigned int size, struct mulle_allocator *allocator)` — zeroes the struct, initializes the mutex, records `size` (asserts `size >= 2`) and `allocator`, and `mulle_allocator_calloc`s the storage.
  - `_done`: `void _mulle_lockingpointermultififo_done( struct mulle_lockingpointermultififo *p)` — destroys the mutex and frees `storage`.
- **Core Operations:**
  - `_write`: `int _mulle_lockingpointermultififo_write( struct mulle_lockingpointermultififo *p, void *pointer)` — under the mutex, pushes `pointer` if there is room. Returns 0 on success; -1 with `errno == EBUSY` if full; -1 with `errno == EINVAL` if `pointer == (void *) ~0`. NULL pointers are rejected via `assert(pointer != NULL)` in debug builds.
  - `_read_barrier`: `void *_mulle_lockingpointermultififo_read_barrier( struct mulle_lockingpointermultififo *p)` — under the mutex, pops the next element, or returns NULL if empty (never blocks). Memory consistency is implied by the mutex.
- **Inspection Functions:**
  - `_get_count`: `unsigned int _mulle_lockingpointermultififo_get_count( struct mulle_lockingpointermultififo *p)` — returns the number of currently queued elements, read under the mutex. (The lock-free variant has no equivalent; a count would be racy by definition.)

## 4. Performance Characteristics

- **Write / read: O(1)** in both variants (no per-element scanning; storage is
  one preallocated ring buffer, indexed by `counter % size`).
- **Lock-free variant:** Uses `mulle_atomic_pointer` read/CAS operations with
  the `NOTYET` sentinel. Under contention the inner CAS loop retries within a
  bounded run of ~32 iterations and then calls `mulle_thread_yield()` before
  looping again. No syscalls or kernel locking are involved; startup/allocation
  overhead comes only from the single `mulle_allocator_calloc` in `_init`.
- **Locking variant:** Every `_write` / `_read_barrier` / `_get_count`
  acquires the mutex. O(1) but with typical mutex contention costs; with many
  threads the shared mutex can become the bottleneck.
- **Memory:** For a queue of capacity `size`, storage is `size` pointers,
  allocated once at `_init` and freed at `_done`. Make the FIFO at least as
  big as the expected number of competing threads; underdimensioning simply
  yields `EBUSY` Write failures, not deadlock.
- **Thread-safety:** Both variants are safe for concurrent use by multiple
  producers and multiple consumers without application-level synchronization.
  `_done` must not run while any other thread still uses the FIFO.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:**
  - Always pair `_mulle_pointermultififo_init( ..., size, allocator)` with
    `_mulle_pointermultififo_done(...)` to avoid leaking the storage.
  - Pass `NULL` as the allocator to use the standard allocator, or any
    `struct mulle_allocator *` for a custom allocation scheme.
  - Sizing: a FIFO must be at least 2 entries; prefer making it at least as
    large as the number of threads expected to contend for it.
  - Choose the locking variant when you also need an accurate element count
    (`_get_count`); choose the lock-free variant when minimizing contention
    matters and no count is required.
  - Use the `_read_barrier` variants whenever the *contents* of the pointed-to
    memory must be visible across threads; the barrier establishes that
    consistency point.
- **Common Pitfalls:**
  - Do not store NULL pointers or `(void *) ~0` (`NOTYET`) pointers; uses of
    these are rejected with `errno == EINVAL` (and NULL additionally trips an
    assert in the locking write).
  - A full FIFO is reported as `-1` with `errno == EBUSY`, not an error from
    your code — it is the queue contract signaling "consumers are taking too
    long". Handle `-1` from `_write` explicitly; both variants are
    non-blocking.
  - An empty read returns a NULL pointer; treat NULL as "empty", never as a
    valid payload.
  - Do not access the `_`-prefixed/internal fields (`storage`, `read`,
    `write`, `n`, `lock`) directly; the public functions are the only
    supported interface.
  - `_mulle_pointermultififo_done` does not synchronize; terminate all
    producer/consumer threads (e.g. `mulle_thread_join`) before calling it.
- **Idiomatic Usage:**
  - Initialize once at the start of the work distribution, have many threads
    call `_write` / `_read_barrier` freely, and tear down after all threads
    have been joined.

## 6. Integration Examples

### Example 1: Single-threaded fill and drain (lock-free variant)

```c
#include <mulle-multififo/mulle-multififo.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>


#define N_ITEMS  100


int  main( int argc, char *argv[])
{
   struct mulle_pointermultififo   fifo;
   void                        *pointer;
   unsigned int                i;
   int                         rval;

   // create the FIFO with capacity for N_ITEMS pointers, default allocator
   _mulle_pointermultififo_init( &fifo, N_ITEMS, NULL);

   // write returns 0, or -1 (errno EBUSY) if the FIFO is full
   for( i = 0; i < N_ITEMS; i++)
   {
      rval = _mulle_pointermultififo_write( &fifo, (void *) (intptr_t) i + 1);
      if( rval == -1)
         fprintf( stderr, "full: %s\n", strerror( errno));
   }

   // read returns the pointer, or NULL if the FIFO is empty
   for( i = 0; i < N_ITEMS; i++)
   {
      pointer = _mulle_pointermultififo_read_barrier( &fifo);
      printf( "%ld\n", (long) (intptr_t) pointer);
   }

   // free the FIFO storage
   _mulle_pointermultififo_done( &fifo);

   return( 0);
}
```

### Example 2: Multi-producer/multi-consumer work distribution

```c
#include <mulle-multififo/mulle-multififo.h>

#include <stdio.h>


#define N_ITEMS         1024
#define N_QUEUE_ITEMS    64
#define N_THREADS         4


static mulle_thread_rval_t   producer( void *arg)
{
   struct mulle_pointermultififo   *fifo = arg;
   int                              i;

   i = 1;
   for(;;)
   {
      // retry while the FIFO is full (non-blocking, returns -1)
      if( _mulle_pointermultififo_write( fifo, (void *) (intptr_t) i) == -1)
         continue;

      if( i == N_ITEMS)
         break;
      ++i;
   }
   mulle_thread_return();
}


static mulle_thread_rval_t   consumer( void *arg)
{
   struct mulle_pointermultififo   *fifo = arg;
   void                             *pointer;

   for(;;)
   {
      // NULL means empty, retry
      pointer = _mulle_pointermultififo_read_barrier( fifo);
      if( ! pointer)
         continue;

      if( pointer == (void *) N_ITEMS)
         break;
   }
   mulle_thread_return();
}


int  main( int argc, char *argv[])
{
   struct mulle_pointermultififo   fifo;
   mulle_thread_t                  produce[ N_THREADS];
   mulle_thread_t                  consume[ N_THREADS];
   int                             i;

   _mulle_pointermultififo_init( &fifo, N_QUEUE_ITEMS, NULL);

   for( i = 0; i < N_THREADS; i++)
      mulle_thread_create( producer, &fifo, &produce[ i]);
   for( i = 0; i < N_THREADS; i++)
      mulle_thread_create( consumer, &fifo, &consume[ i]);

   for( i = 0; i < N_THREADS; i++)
      mulle_thread_join( produce[ i]);
   for( i = 0; i < N_THREADS; i++)
      mulle_thread_join( consume[ i]);

   // only safe once all threads are done
   _mulle_pointermultififo_done( &fifo);

   return( 0);
}
```

### Example 3: Locking variant with element counting

```c
#include <mulle-multififo/mulle-multififo.h>

#include <stdio.h>


#define N_ITEMS  8


int  main( int argc, char *argv[])
{
   struct mulle_lockingpointermultififo   fifo;
   void                                 *pointer;
   unsigned int                         n;
   int                                  i;

   _mulle_lockingpointermultififo_init( &fifo, N_ITEMS, NULL);

   for( i = 0; i < N_ITEMS; i++)
   {
      // returns 0 on success, -1 (errno EBUSY) when full
      _mulle_lockingpointermultififo_write( &fifo, (void *) (intptr_t) i);
   }

   // only the locking variant can report the queue length safely
   n = _mulle_lockingpointermultififo_get_count( &fifo);
   printf( "%u items in fifo\n", n);

   pointer = _mulle_lockingpointermultififo_read_barrier( &fifo);
   printf( "%ld\n", (long) (intptr_t) pointer);

   _mulle_lockingpointermultififo_done( &fifo);

   return( 0);
}
```

## 7. Dependencies

Direct mulle-sde dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-thread` — atomic pointer operations (CAS), the memory barrier and
  the mutex (`mulle_thread_mutex_t`); used by both FIFO variants.
- `mulle-allocator` — `mulle_allocator_calloc` / `mulle_allocator_free` for
  the ring-buffer storage; accepts a caller-provided allocator.