# mulle-fifo Library Documentation for AI
<!-- Keywords: lock-free, queue, spsc, non-blocking, atomics -->

## 1. Introduction & Purpose

mulle-fifo is a bounded, non-blocking producer/consumer FIFO (First-In-First-Out)
queue for `void *` pointers. It provides a collection of fixed-size FIFOs
(4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 entries) plus a
dynamically-sized variant. It is designed for exactly one producer thread and
one consumer thread (SPSC). This is a foundational utility in the
mulle-concurrent ecosystem for building producer-consumer patterns and task
hand-off.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Single Producer/Single Consumer:** Exactly one thread may write and one
  thread may read. Calls from additional threads are forbidden; they are not
  merely serialized internally, they corrupt the state.

- **Non-Blocking:** Both read and write return immediately; they never block,
  never grow the storage, and never allocate.

- **Fixed or Dynamic Size:** Pre-sized FIFOs with storage embedded in the
  struct, or dynamic sizing with a caller-supplied allocator.

- **No NULL Support:** NULL cannot be stored (a write of NULL fails with
  `-2`); a read returning NULL means the FIFO is empty.

- **Relaxed Atomics:** The internal counter and storage slots are accessed
  with sequentially consistent atomic operations (see mulle-thread). A
  successful `write` publishes both the pointer value and the contents of
  the pointed-to object; a subsequent `read` that observes the pointer may
  safely dereference it. No additional barriers are required on either side.

**Naming convention:** Functions prefixed with an underscore do not check
their fifo argument for NULL. The dynamic FIFO additionally offers
same-named wrappers without the underscore that tolerate a NULL fifo. The
fixed FIFOs only have the underscore-prefixed functions.

## 3. Core API & Data Structures

### 3.1 Fixed-Size FIFOs

Available sizes: 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
entries. In the names below, replace `N` with the size.

#### Type

**`struct mulle__pointerfifoN`** (e.g. `struct mulle__pointerfifo64`)

- **Purpose:** Fixed-size FIFO queue holding void pointers.
- **Internal:**
  - `n`: Atomic count of entries.
  - `write`: Write position (producer only).
  - `read`: Read position (consumer only).
  - `storage`: Array of N atomic pointers, embedded in the struct.

#### Lifecycle Functions

**`void _mulle__pointerfifoN_init(struct mulle__pointerfifoN *p)`**

- **Purpose:** Initialize FIFO for use. Must complete before producer or
  consumer start.

**`void _mulle__pointerfifoN_done(struct mulle__pointerfifoN *p)`**

- **Purpose:** Cleanup. This is a no-op, as the storage is embedded in the
  struct.

#### Read Operations

**`void *_mulle__pointerfifoN_read(struct mulle__pointerfifoN *p)`**

- **Purpose:** Read one pointer from FIFO (consumer side).
- **Returns:** Pointer (non-NULL), or NULL if FIFO empty.
- **Thread Safety:** Consumer-only; only one thread may call.

#### Write Operations

**`int _mulle__pointerfifoN_write(struct mulle__pointerfifoN *p, void *pointer)`**

- **Purpose:** Write one pointer to FIFO (producer side).
- **Parameters:**
  - `p`: FIFO queue.
  - `pointer`: Non-NULL pointer to enqueue.
- **Returns:** 0 on success, -1 if FIFO full, -2 if `pointer` is NULL.
- **Thread Safety:** Producer-only; only one thread may call.

#### Inspection

**`unsigned int _mulle__pointerfifoN_get_count(struct mulle__pointerfifoN *p)`**

- **Purpose:** Get the number of entries in the FIFO.
- **Thread Safety:** Safe to call from any thread.
- **Note:** Single atomic read of the shared counter; exact at the moment of
  the read, but possibly stale by the time the caller acts on it.

### 3.2 Dynamic-Size FIFO

#### Type

**`struct mulle_pointerfifo`**

- **Purpose:** Runtime-sized FIFO queue.
- **Internal:** Like the fixed FIFO, plus `size`, `allocator` and a
  heap-allocated `storage` array.

#### Lifecycle Functions

**`void _mulle_pointerfifo_init(struct mulle_pointerfifo *p, unsigned int size, struct mulle_allocator *allocator)`**

- **Purpose:** Initialize dynamic FIFO with room for `size` pointers.
- **Parameters:**
  - `p`: Uninitialized FIFO.
  - `size`: Capacity (>= 2, enforced with `assert`).
  - `allocator`: Memory allocator for storage; NULL selects the default
    allocator. Allocation failure follows the mulle-allocator contract: the
    allocator's `fail` function is called, which by default aborts.

**`void mulle_pointerfifo_init(struct mulle_pointerfifo *p, unsigned int size, struct mulle_allocator *allocator)`** (NULL-safe)

- **Purpose:** Wrapper that tolerates a NULL `p`.

**`void _mulle_pointerfifo_done(struct mulle_pointerfifo *p)`**

- **Purpose:** Free FIFO storage. Pointers still in the FIFO are not freed.
  Safe to call more than once. Re-initialize before reuse.

**`void mulle_pointerfifo_done(struct mulle_pointerfifo *p)`** (NULL-safe)

- **Purpose:** Wrapper that tolerates a NULL `p`.

#### Read/Write Operations

**`void *_mulle_pointerfifo_read(struct mulle_pointerfifo *p)`**

- **Purpose:** Read pointer (consumer).
- **Returns:** Pointer or NULL if empty.

**`void *mulle_pointerfifo_read(struct mulle_pointerfifo *p)`** (NULL-safe)

- **Purpose:** Wrapper that returns NULL for a NULL `p`.

**`int _mulle_pointerfifo_write(struct mulle_pointerfifo *p, void *pointer)`**

- **Purpose:** Write pointer (producer).
- **Returns:** 0 on success, -1 if full, -2 if `pointer` is NULL.

**`int mulle_pointerfifo_write(struct mulle_pointerfifo *p, void *pointer)`** (NULL-safe)

- **Purpose:** Wrapper that returns -1 for a NULL `p`.

The dynamic FIFO has the same publication guarantees as the fixed FIFO.

#### Inspection

**`unsigned int _mulle_pointerfifo_get_count(struct mulle_pointerfifo *p)`**

- **Purpose:** Get the number of entries (same semantics as the fixed
  `get_count`).

**`unsigned int mulle_pointerfifo_get_count(struct mulle_pointerfifo *p)`** (NULL-safe)

- **Purpose:** Wrapper that returns 0 for a NULL `p`.

## 4. Performance Characteristics

- **Read/Write Time:** O(1) per operation; no locks, no system calls, no
  allocation.
- **Memory:** Fixed FIFO: the storage (N pointers) is embedded in the struct.
  Dynamic: one allocation of `size` pointers at init.
- **Contention:** The shared counter `n` is updated atomically by both sides
  on every operation, which causes cache-line hand-over between the two
  threads. This is a known design limitation for an SPSC queue.
- **Latency:** Non-blocking; no OS scheduler involvement.

**Scalability:**
- Single pair (1 producer, 1 consumer): supported configuration.
- Multiple producers/consumers: forbidden; use separate FIFOs or external
  locking.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Enforce Single Producer/Consumer:** Only one thread may call write, one
   may call read. `init` before the threads start, `done` after they stop.

2. **Check Return Values:** Writes fail with -1 when full; decide on a
   retry/backoff or drop policy. Writes of NULL fail with -2.

3. **Size Appropriately:** The FIFO never grows. Pick a capacity that
   absorbs your burst, or handle the full case.

4. **Drain Before Done:** Remaining pointers are not freed by `done`.

5. **Publication:** A successful `write` publishes both the pointer and the
   contents of the pointed-to object. A subsequent `read` that returns the
   pointer may safely dereference it. No additional barriers or special
   read variants are needed.

### Common Pitfalls:

1. **Storing NULL:** Rejected with -2; use a sentinel or wrapper if you need
   an "empty" value.

2. **Multiple Threads per Side:** Corrupts state; this is a contract
   violation, not something the FIFO detects.

3. **Ignoring Full Writes:** The FIFO never blocks; a dropped write is gone
   unless the caller retries.

4. **Memory Ordering:** All internal atomics are sequentially consistent.
   Dereference of dequeued pointers is safe without additional barriers.

5. **Use After Done:** Dynamic `done` frees the storage. It is idempotent,
   but reading/writing a finished FIFO is an error; re-initialize first.

## 6. Integration Examples

### Example 1: Basic Fixed FIFO (64 entries)

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

struct mulle__pointerfifo64   work_queue;

int   main( void)
{
   char   *task;

   _mulle__pointerfifo64_init( &work_queue);

   if( _mulle__pointerfifo64_write( &work_queue, "task1") == 0)
      printf( "queued\n");

   while( (task = _mulle__pointerfifo64_read( &work_queue)) != NULL)
      printf( "Processing: %s\n", task);

   _mulle__pointerfifo64_done( &work_queue);

   return( 0);
}
```

### Example 2: Dynamic FIFO with Runtime Capacity

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_pointerfifo   event_queue;
   void                       *event;
   unsigned int               i;

   mulle_pointerfifo_init( &event_queue, 1000, NULL);

   for( i = 1; i <= 100; i++)
   {
      if( mulle_pointerfifo_write( &event_queue, (void *) (uintptr_t) i) != 0)
      {
         fprintf( stderr, "Queue full at event %u\n", i);
         break;
      }
   }

   printf( "Queued: %u events\n", mulle_pointerfifo_get_count( &event_queue));

   while( (event = mulle_pointerfifo_read( &event_queue)) != NULL)
      printf( "Event: %lu\n", (unsigned long) (uintptr_t) event);

   mulle_pointerfifo_done( &event_queue);

   return( 0);
}
```

### Example 3: Producer/Consumer Threads

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

static struct mulle__pointerfifo128   queue;

static mulle_thread_rval_t   producer( void *arg)
{
   intptr_t   i;

   for( i = 1; i <= 20; i++)
      while( _mulle__pointerfifo128_write( &queue, (void *) i) == -1)
         mulle_thread_yield();

   mulle_thread_return();
}

int   main( void)
{
   mulle_thread_t   producer_thread;
   void             *pointer;
   unsigned int     count;

   _mulle__pointerfifo128_init( &queue);
   mulle_thread_create( producer, NULL, &producer_thread);

   count = 0;
   while( count < 20)
   {
      pointer = _mulle__pointerfifo128_read( &queue);
      if( ! pointer)
      {
         mulle_thread_yield();
         continue;
      }
      printf( "Got %ld\n", (long) (intptr_t) pointer);
      ++count;
   }

   mulle_thread_join( producer_thread);
   _mulle__pointerfifo128_done( &queue);

   return( 0);
}
```

### Example 4: Publication Safety (pointer dereference)

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

struct message
{
   char   text[ 64];
};

static struct mulle__pointerfifo32   queue;
static struct message                messages[ 8];

void   send_messages( void)
{
   unsigned int   i;

   for( i = 0; i < 8; i++)
   {
      snprintf( messages[ i].text, sizeof( messages[ i].text),
                "message %u", i);
      while( _mulle__pointerfifo32_write( &queue, &messages[ i]) == -1)
         mulle_thread_yield();
   }
}

void   receive_messages( void)
{
   struct message   *message;
   unsigned int     count;

   count = 0;
   while( count < 8)
   {
      /* plain read: seq_cst atomics guarantee pointee visibility */
      message = _mulle__pointerfifo32_read( &queue);
      if( ! message)
      {
         mulle_thread_yield();
         continue;
      }
      printf( "Received: %s\n", message->text);
      ++count;
   }
}
```

### Example 5: Monitoring Queue Status

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

static struct mulle__pointerfifo256   task_queue;

int   main( void)
{
   void          *task;
   unsigned int  i;

   _mulle__pointerfifo256_init( &task_queue);

   for( i = 1; i <= 100; i++)
   {
      if( _mulle__pointerfifo256_write( &task_queue, (void *) (uintptr_t) i) == 0)
      {
         if( _mulle__pointerfifo256_get_count( &task_queue) % 10 == 0)
            printf( "Queue depth: %u\n",
                    _mulle__pointerfifo256_get_count( &task_queue));
      }
      else
      {
         printf( "Queue full at task %u\n", i);
         break;
      }
   }

   while( (task = _mulle__pointerfifo256_read( &task_queue)) != NULL)
      printf( "Processed %lu, remaining: %u\n",
              (unsigned long) (uintptr_t) task,
              _mulle__pointerfifo256_get_count( &task_queue));

   _mulle__pointerfifo256_done( &task_queue);

   return( 0);
}
```

## 7. Dependencies

Direct dependencies (from `.mulle/etc/sourcetree/config`):
- `mulle-thread` (>= 4.10.0): sequentially consistent atomic operations, memory barriers
- `mulle-allocator` (>= 8.1.0): storage allocation for the dynamic FIFO

Minimum versions are enforced by `src/reflect/_mulle-fifo-versioncheck.h`
(current as of version 0.1.8). `MULLE_C_GLOBAL`, `MULLE_C_UNUSED` and friends
are provided transitively via `mulle-thread`; there is no direct dependency on
`mulle-c11`.
