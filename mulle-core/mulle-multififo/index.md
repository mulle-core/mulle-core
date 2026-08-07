# mulle-multififo Library Documentation for AI
<!-- Keywords: lock-free, multi-queue -->

## 1. Introduction & Purpose

mulle-multififo is a multi-producer/multi-consumer FIFO queue for thread-safe work distribution. Unlike mulle-fifo (single producer/consumer), multififo supports arbitrary numbers of threads pushing and popping simultaneously using internal locking or lock-free mechanisms. It provides runtime-sized queues with optional memory barriers for safe pointer handling across threads. This is a key utility in the mulle-concurrent ecosystem for thread pools, work queues, and producer-consumer patterns with multiple threads.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Multi-Threaded:** Multiple readers and writers safe simultaneously.
- **Internal Synchronization:** Handles locking internally; user code synchronization-free.
- **Dual Variants:** Locking and lock-free implementations available.
- **No NULL Support:** Cannot store NULL or ~0 (reserved sentinel values).
- **Non-Blocking:** Write returns -1 if full; read returns NULL if empty.
- **Memory Barriers:** Optional barrier variants for external pointer safety.

## 3. Core API & Data Structures

#### Type

**`struct mulle_pointermultififo`**

- **Purpose:** Multi-producer/consumer FIFO queue.
- **Thread-Safe:** Multiple threads can read/write concurrently.

#### Lifecycle

**`void _mulle_pointermultififo_init(struct mulle_pointermultififo *p, unsigned int size, struct mulle_allocator *allocator)`**

- **Purpose:** Initialize multififo.
- **Parameters:**
  - `p`: Uninitialized FIFO.
  - `size`: Capacity (recommend >= number of threads).
  - `allocator`: Memory allocator.

**`void _mulle_pointermultififo_done(struct mulle_pointermultififo *p)`**

- **Purpose:** Destroy multififo and free storage.

#### Read Operations

**`void *_mulle_pointermultififo_read_barrier(struct mulle_pointermultififo *p)`**

- **Purpose:** Read pointer (any thread) with memory barrier.
- **Returns:** Pointer (non-NULL), or NULL if empty.
- **Note:** The memory barrier ensures that the memory pointed to by the returned pointer is valid.

#### Write Operations

**`int _mulle_pointermultififo_write(struct mulle_pointermultififo *p, void *pointer)`**

- **Purpose:** Write pointer (any thread).
- **Returns:** 0 on success, -1 if full.
- **Note:** Will not block; returns immediately with -1 if the FIFO is full.

#### Inspection

The API does not currently provide direct inspection functions. Queue size can be estimated but is subject to race conditions in multi-threaded environments.

## 4. Performance Characteristics

- **Read:** O(1) amortized; contention causes retry.
- **Write:** O(1) amortized; full queue causes failure.
- **Scalability:** Performance degrades with thread count (contention).

## 5. Integration Examples

### Example 1: Basic Multi-Threaded Producer/Consumer

```c
#include <mulle-multififo/mulle-multififo.h>
#include <mulle-thread/mulle-thread.h>
#include <stdio.h>

struct mulle_pointermultififo work_queue;

void *producer(void *arg) {
    int thread_id = (intptr_t)arg;
    
    for (int i = 0; i < 5; i++) {
        void *work = (void *)(intptr_t)(thread_id * 100 + i);
        
        while (_mulle_pointermultififo_write(&work_queue, work) != 0) {
            // Retry if full
        }
        
        printf("Producer %d: queued %p\n", thread_id, work);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    int thread_id = (intptr_t)arg;
    int consumed = 0;
    
    while (consumed < 10) {
        void *work = _mulle_pointermultififo_read_barrier(&work_queue);
        if (work) {
            printf("Consumer %d: processing %p\n", thread_id, work);
            consumed++;
        }
    }
    
    return NULL;
}

int main() {
    _mulle_pointermultififo_init(&work_queue, 32, NULL);
    
    mulle_thread_t prod1, prod2, cons1, cons2;
    
    mulle_thread_create(&prod1, producer, (void *)1);
    mulle_thread_create(&prod2, producer, (void *)2);
    mulle_thread_create(&cons1, consumer, (void *)1);
    mulle_thread_create(&cons2, consumer, (void *)2);
    
    mulle_thread_join(prod1, NULL);
    mulle_thread_join(prod2, NULL);
    mulle_thread_join(cons1, NULL);
    mulle_thread_join(cons2, NULL);
    
    _mulle_pointermultififo_done(&work_queue);
    
    return 0;
}
```

### Example 2: Thread Pool Work Distribution

```c
#include <mulle-multififo/mulle-multififo.h>
#include <mulle-thread/mulle-thread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int job_id;
    int data;
} Job;

struct mulle_pointermultififo job_queue;

void *worker(void *arg) {
    int worker_id = (intptr_t)arg;
    
    while (1) {
        Job *job = (Job *)_mulle_pointermultififo_read_barrier(&job_queue);
        
        if (!job) continue;
        
        printf("Worker %d: Processing job %d (data=%d)\n", 
               worker_id, job->job_id, job->data);
        
        free(job);
    }
    
    return NULL;
}

int main() {
    _mulle_pointermultififo_init(&job_queue, 100, NULL);
    
    // Create 4 worker threads
    mulle_thread_t workers[4];
    for (int i = 0; i < 4; i++) {
        mulle_thread_create(&workers[i], worker, (void *)i);
    }
    
    // Queue jobs
    for (int i = 0; i < 20; i++) {
        Job *job = malloc(sizeof(Job));
        job->job_id = i;
        job->data = i * 10;
        
        while (_mulle_pointermultififo_write(&job_queue, job) != 0) {
            // Queue full, retry
        }
    }
    
    // In real code, signal workers to stop
    // mulle_thread_join(...);
    // _mulle_pointermultififo_done(&job_queue);
    
    return 0;
}
```

## 6. Dependencies

- `mulle-thread`: Atomic operations and synchronization
- `mulle-allocator`: Memory allocation
- `mulle-c11`: Compatibility macros
