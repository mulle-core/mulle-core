# mulle-fifo Library Documentation for AI
<!-- Keywords: lock-free, queue -->

## 1. Introduction & Purpose

mulle-fifo is a high-performance, lock-free producer/consumer FIFO (First-In-First-Out) queue for multi-threaded applications. It provides a collection of fixed-size FIFOs (8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 entries) plus a dynamically-sized variant. Designed for dual-thread configurations with one producer and one consumer, it minimizes synchronization overhead while maintaining thread-safe pointer passing. This is a foundational utility in the mulle-concurrent ecosystem for building producer-consumer patterns, task queues, and work stealing schedulers.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Single Producer/Single Consumer:** Optimized for exactly one thread writing and one reading, avoiding complex locking.

- **Lock-Free Operations:** Uses atomic compare-and-swap and memory barriers instead of mutexes for minimal overhead.

- **Fixed or Dynamic Size:** Pre-sized FIFOs for compile-time optimization, or dynamic sizing for runtime flexibility.

- **Non-Blocking:** Both read and write operations return immediately; never block the caller.

- **No NULL Support:** Cannot store NULL pointers (NULL indicates empty).

- **Memory Barriers:** Provides both fast and barrier-protected versions for different usage scenarios.

- **Atomic Counting:** Thread-safe count queries via atomic operations.

## 3. Core API & Data Structures

### 3.1 Fixed-Size FIFOs

Available sizes: 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 entries.

#### Type

**`struct mulle__pointerfifoN`** (where N is size, e.g., `mulle__pointerfifo64`)

- **Purpose:** Fixed-size FIFO queue holding void pointers.
- **Internal:**
  - `n`: Atomic count of entries.
  - `write`: Write position (producer only).
  - `read`: Read position (consumer only).
  - `storage`: Array of atomic pointers.

#### Lifecycle Functions

**`void _mulle__pointerfifoN_init(struct mulle__pointerfifoN *p)`**

- **Purpose:** Initialize FIFO for use.
- **Idempotent:** Safe to call on uninitialized or reused memory.

**`void _mulle__pointerfifoN_done(struct mulle__pointerfifoN *p)`**

- **Purpose:** Cleanup (typically no-op; provided for API completeness).

#### Read Operations

**`void *_mulle__pointerfifoN_read(struct mulle__pointerfifoN *p)`**

- **Purpose:** Read one pointer from FIFO (consumer side).
- **Returns:** Pointer (non-NULL), or NULL if FIFO empty.
- **Barrier:** No memory barrier (fast path).
- **Thread Safety:** Consumer-only; only one thread may call.

**`void *_mulle__pointerfifoN_read_barrier(struct mulle__pointerfifoN *p)`**

- **Purpose:** Read with memory barrier.
- **Use:** When pointer points to data that was written from another thread.

#### Write Operations

**`int _mulle__pointerfifoN_write(struct mulle__pointerfifoN *p, void *pointer)`**

- **Purpose:** Write one pointer to FIFO (producer side).
- **Parameters:**
  - `p`: FIFO queue.
  - `pointer`: Non-NULL pointer to enqueue.
- **Returns:** 0 on success, -1 if FIFO full.
- **Barrier:** No memory barrier (fast path).
- **Thread Safety:** Producer-only; only one thread may call.

**`int _mulle__pointerfifoN_write_barrier(struct mulle__pointerfifoN *p, void *pointer)`**

- **Purpose:** Write with memory barrier.
- **Use:** When pointer points to data that will be read from another thread.

#### Inspection

**`unsigned int _mulle__pointerfifoN_get_count(struct mulle__pointerfifoN *p)`**

- **Purpose:** Get approximate number of entries in FIFO.
- **Thread Safety:** Safe to call from either thread.
- **Note:** Count is approximate due to atomic operations; may be off-by-one temporarily.

### 3.2 Dynamic-Size FIFO

#### Type

**`struct mulle_pointerfifo`**

- **Purpose:** Runtime-sized FIFO queue.
- **Flexibility:** Size specified at initialization.

#### Lifecycle Functions

**`void _mulle_pointerfifo_init(struct mulle_pointerfifo *p, unsigned int size, struct mulle_allocator *allocator)`**

- **Purpose:** Initialize dynamic FIFO with specified size.
- **Parameters:**
  - `p`: Uninitialized FIFO.
  - `size`: Capacity (>= 2).
  - `allocator`: Memory allocator for storage.

**`void mulle_pointerfifo_init(struct mulle_pointerfifo *p, unsigned int size, struct mulle_allocator *allocator)`** (NULL-safe)

- **Purpose:** Safe wrapper that checks NULL.

**`void _mulle_pointerfifo_done(struct mulle_pointerfifo *p)`**

- **Purpose:** Free FIFO storage.
- **Must Call:** To avoid memory leaks.

**`void mulle_pointerfifo_done(struct mulle_pointerfifo *p)`** (NULL-safe)

- **Purpose:** Safe wrapper that checks NULL.

#### Read/Write Operations

**`void *_mulle_pointerfifo_read(struct mulle_pointerfifo *p)`**

- **Purpose:** Read pointer (consumer).
- **Returns:** Pointer or NULL if empty.

**`void *_mulle_pointerfifo_read_barrier(struct mulle_pointerfifo *p)`**

- **Purpose:** Read with memory barrier.

**`int _mulle_pointerfifo_write(struct mulle_pointerfifo *p, void *pointer)`**

- **Purpose:** Write pointer (producer).
- **Returns:** 0 on success, -1 if full.

**`int _mulle_pointerfifo_write_barrier(struct mulle_pointerfifo *p, void *pointer)`**

- **Purpose:** Write with memory barrier.

#### Inspection

**`unsigned int _mulle_pointerfifo_get_count(struct mulle_pointerfifo *p)`**

- **Purpose:** Get approximate entry count.

## 4. Performance Characteristics

- **Read Time:** O(1) per operation, typically 2-3 CPU cycles (uncontended).
- **Write Time:** O(1) per operation, typically 2-3 CPU cycles (uncontended).
- **Memory:** Fixed FIFO: Compile-time known; Dynamic: allocator-dependent.
- **Contention:** Minimal with single producer/consumer; count updates use atomic CAS.
- **Latency:** Non-blocking; no OS scheduler involvement.

**Scalability:**
- Single pair (1 producer, 1 consumer): Optimal performance.
- Multiple producers/consumers: Use separate FIFO pairs or external locking.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Enforce Single Producer/Consumer:** Only one thread should call write, one should call read.

2. **Use Barrier Variants When Needed:** If pointer references external data, use `*_barrier` variants.

3. **Check Return Values:** Always check write return for full conditions; handle gracefully.

4. **Size Appropriately:** Choose fixed size that won't overflow; or use dynamic with generous capacity.

5. **Drain Before Done:** Ensure all entries consumed before calling `*_done()`.

### Common Pitfalls:

1. **Storing NULL:** Cannot store NULL; use sentinel or wrapper structure.

2. **Multiple Threads per Side:** Multiple producers or consumers corrupt state; use mutex wrapping if needed.

3. **Ignoring Full Writes:** Silently discarded writes if FIFO full; queue can deadlock waiting.

4. **Memory Ordering Confusion:** Forgetting `*_barrier` when pointer references external memory causes races.

5. **Size Too Small:** Fixed-size FIFO with insufficient capacity drops data.

## 6. Integration Examples

### Example 1: Basic Fixed FIFO (64 entries)

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>
#include <string.h>

struct mulle__pointerfifo64 work_queue;

void producer_task(void) {
    const char *tasks[] = {"task1", "task2", "task3", NULL};
    
    _mulle__pointerfifo64_init(&work_queue);
    
    for (int i = 0; tasks[i]; i++) {
        char *task = malloc(strlen(tasks[i]) + 1);
        strcpy(task, tasks[i]);
        
        if (_mulle__pointerfifo64_write(&work_queue, task) != 0) {
            fprintf(stderr, "Queue full!\n");
            free(task);
        }
    }
}

void consumer_task(void) {
    char *task;
    
    while ((task = _mulle__pointerfifo64_read(&work_queue)) != NULL) {
        printf("Processing: %s\n", task);
        free(task);
    }
    
    _mulle__pointerfifo64_done(&work_queue);
}

int main() {
    producer_task();
    consumer_task();
    return 0;
}
```

### Example 2: Dynamic FIFO with Larger Capacity

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>

struct mulle_pointerfifo event_queue;

int main() {
    mulle_pointerfifo_init(&event_queue, 1000, NULL);
    
    // Enqueue events
    for (int i = 1; i <= 100; i++) {
        void *event = (void *)(intptr_t)i;
        
        if (mulle_pointerfifo_write(&event_queue, event) != 0) {
            fprintf(stderr, "Queue full at event %d\n", i);
            break;
        }
    }
    
    printf("Queued: %u events\n", 
           mulle_pointerfifo_get_count(&event_queue));
    
    // Dequeue and process
    void *event;
    int count = 0;
    while ((event = mulle_pointerfifo_read(&event_queue)) != NULL) {
        int event_id = (intptr_t)event;
        printf("Event: %d\n", event_id);
        count++;
    }
    
    printf("Processed: %d events\n", count);
    
    mulle_pointerfifo_done(&event_queue);
    
    return 0;
}
```

### Example 3: Work Stealing Pattern

```c
#include <mulle-fifo/mulle-fifo.h>
#include <mulle-thread/mulle-thread.h>
#include <stdio.h>

typedef struct {
    int work_id;
    int complexity;
} WorkItem;

struct mulle__pointerfifo128 work_queue;

void *worker_thread(void *arg) {
    int thread_id = (intptr_t)arg;
    WorkItem *work;
    int processed = 0;
    
    while ((work = _mulle__pointerfifo128_read(&work_queue)) != NULL) {
        printf("Thread %d: Processing work %d (complexity %d)\n",
               thread_id, work->work_id, work->complexity);
        processed++;
        free(work);
    }
    
    printf("Thread %d: Processed %d items\n", thread_id, processed);
    
    return NULL;
}

int main() {
    _mulle__pointerfifo128_init(&work_queue);
    
    // Enqueue work
    for (int i = 1; i <= 20; i++) {
        WorkItem *work = malloc(sizeof(WorkItem));
        work->work_id = i;
        work->complexity = (i % 5) + 1;
        
        _mulle__pointerfifo128_write(&work_queue, work);
    }
    
    printf("Queued %u work items\n", 
           _mulle__pointerfifo128_get_count(&work_queue));
    
    // Simulate consumer
    worker_thread((void *)0);
    
    _mulle__pointerfifo128_done(&work_queue);
    
    return 0;
}
```

### Example 4: Memory Barrier Usage

```c
#include <mulle-fifo/mulle-fifo.h>
#include <string.h>

typedef struct {
    char data[64];
} Message;

struct mulle__pointerfifo32 message_queue;

void send_message(const char *msg) {
    Message *message = malloc(sizeof(Message));
    strncpy(message->data, msg, sizeof(message->data) - 1);
    
    // Use barrier because pointer references external data
    if (_mulle__pointerfifo32_write_barrier(&message_queue, message) != 0) {
        free(message);
    }
}

void receive_message(void) {
    Message *message;
    
    // Use barrier because we access data pointed to by dequeued pointer
    while ((message = _mulle__pointerfifo32_read_barrier(&message_queue)) != NULL) {
        printf("Received: %s\n", message->data);
        free(message);
    }
}

int main() {
    _mulle__pointerfifo32_init(&message_queue);
    
    send_message("Hello");
    send_message("World");
    
    receive_message();
    
    _mulle__pointerfifo32_done(&message_queue);
    
    return 0;
}
```

### Example 5: Monitoring Queue Status

```c
#include <mulle-fifo/mulle-fifo.h>
#include <stdio.h>
#include <unistd.h>

struct mulle__pointerfifo256 task_queue;

int main() {
    _mulle__pointerfifo256_init(&task_queue);
    
    // Simulate task generation
    for (int i = 0; i < 100; i++) {
        void *task = (void *)(intptr_t)(i + 1);
        
        if (_mulle__pointerfifo256_write(&task_queue, task) == 0) {
            unsigned int count = _mulle__pointerfifo256_get_count(&task_queue);
            if (count % 10 == 0) {
                printf("Queue depth: %u\n", count);
            }
        } else {
            printf("Queue full at task %d\n", i + 1);
            break;
        }
    }
    
    // Simulate task consumption
    while (1) {
        void *task = _mulle__pointerfifo256_read(&task_queue);
        if (!task) break;
        
        unsigned int remaining = _mulle__pointerfifo256_get_count(&task_queue);
        printf("Processed task %p, remaining: %u\n", task, remaining);
    }
    
    _mulle__pointerfifo256_done(&task_queue);
    
    return 0;
}
```

## 7. Dependencies

Direct dependencies:
- `mulle-thread`: Atomic operations and memory barriers
- `mulle-c11`: C11 compatibility and utility macros
