# mulle-linkedlist Library Documentation for AI
<!-- Keywords: lock-free, linked-list -->

## 1. Introduction & Purpose

mulle-linkedlist provides two linked list implementations: a single-threaded intrusive linked list (`mulle_linkedlist`) and a lock-free concurrent variant (`mulle_concurrent_linkedlist`). Intrusive design means list nodes are embedded directly in user structures, eliminating allocation overhead. The concurrent variant uses atomic operations and ABA-safe pointers for thread-safe head manipulation without locks. This is a foundational utility in the mulle-concurrent ecosystem used for implementing lock-free data structures and internal runtime management.

## 2. Key Concepts & Design Philosophy

**Intrusive Lists:**
- List node embedded in user structure (not separate allocation).
- No dynamic memory for list management.
- User responsible for struct layout.

**Concurrent Variant:**
- Lock-free head insertion via CAS.
- Each thread can push/pop without locking.
- Uses ABA protection for safe concurrent access.

**Design Principles:**

- **Minimal Overhead:** No wrapper allocations; O(1) space per element.
- **Simplicity:** Intentionally barebones for maximum control.
- **Composability:** Build complex structures from simple primitives.

## 3. Core API & Data Structures

### 3.1 Single-Threaded List

#### Type

**`struct _mulle_linkedlistentry`**

- **Purpose:** List node (embed at start of user structure).
- **Fields:**
  - `_next`: Pointer to next entry.

#### Operations

**`void _mulle_linkedlistentry_chain(struct _mulle_linkedlistentry **head, struct _mulle_linkedlistentry *entry)`**

- **Purpose:** Insert entry at head (O(1)).
- **Parameters:**
  - `head`: Pointer to list head.
  - `entry`: Entry to insert.

**`struct _mulle_linkedlistentry *_mulle_linkedlistentry_unchain(struct _mulle_linkedlistentry **head)`**

- **Purpose:** Remove and return head entry.
- **Returns:** Former head (NULL if empty).

**`void _mulle_linkedlistentry_walk(struct _mulle_linkedlistentry *next, mulle_linkedlistentry_walk_callback_t *callback, void *userinfo)`**

- **Purpose:** Call callback for each entry.
- **Callback Signature:** `void (*callback)(void *userinfo, struct _mulle_linkedlistentry *entry)`

### 3.2 Concurrent List

#### Type

**`struct mulle_concurrent_linkedlistentry`**

- **Purpose:** Lock-free list node.
- **Uses:** Atomic pointers for safe CAS operations.

#### Operations

**`void mulle_concurrent_linkedlistentry_push(struct mulle_concurrent_linkedlistentry **head, struct mulle_concurrent_linkedlistentry *entry)`**

- **Purpose:** Lock-free insert at head.
- **Thread-Safe:** Multiple threads can push simultaneously.

**`struct mulle_concurrent_linkedlistentry *mulle_concurrent_linkedlistentry_pop(struct mulle_concurrent_linkedlistentry **head)`**

- **Purpose:** Lock-free remove from head (with retry).
- **Returns:** Former head (NULL if empty).

## 4. Performance Characteristics

- **Chain:** O(1).
- **Unchain:** O(1).
- **Walk:** O(n).
- **Concurrent Push/Pop:** O(1) with CAS retry; wait-free under low contention.

## 5. Integration Examples

### Example 1: Single-Threaded List

```c
#include <mulle-linkedlist/mulle-linkedlist.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    struct _mulle_linkedlistentry entry;
    int value;
    char name[32];
} Item;

int main() {
    struct _mulle_linkedlistentry *list = NULL;
    
    // Create and chain items
    Item *item1 = malloc(sizeof(Item));
    item1->value = 1;
    sprintf(item1->name, "Item 1");
    _mulle_linkedlistentry_chain(&list, &item1->entry);
    
    Item *item2 = malloc(sizeof(Item));
    item2->value = 2;
    sprintf(item2->name, "Item 2");
    _mulle_linkedlistentry_chain(&list, &item2->entry);
    
    // Unchain (LIFO order)
    struct _mulle_linkedlistentry *entry;
    while ((entry = _mulle_linkedlistentry_unchain(&list))) {
        Item *item = (Item *)entry;
        printf("Item: %s (value: %d)\n", item->name, item->value);
        free(item);
    }
    
    return 0;
}
```

### Example 2: Lock-Free Concurrent List

```c
#include <mulle-linkedlist/mulle-linkedlist.h>
#include <mulle-thread/mulle-thread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    struct mulle_concurrent_linkedlistentry entry;
    int task_id;
} Task;

struct mulle_concurrent_linkedlistentry *task_queue = NULL;

void *producer(void *arg) {
    int thread_id = (intptr_t)arg;
    
    for (int i = 0; i < 10; i++) {
        Task *task = malloc(sizeof(Task));
        task->task_id = thread_id * 100 + i;
        mulle_concurrent_linkedlistentry_push(&task_queue, &task->entry);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    int consumed = 0;
    
    while (consumed < 20) {
        struct mulle_concurrent_linkedlistentry *entry = 
            mulle_concurrent_linkedlistentry_pop(&task_queue);
        
        if (entry) {
            Task *task = (Task *)entry;
            printf("Processing task: %d\n", task->task_id);
            free(task);
            consumed++;
        }
    }
    
    return NULL;
}

int main() {
    mulle_thread_t prod1, prod2, cons;
    
    mulle_thread_create(&prod1, producer, (void *)1);
    mulle_thread_create(&prod2, producer, (void *)2);
    mulle_thread_create(&cons, consumer, NULL);
    
    mulle_thread_join(prod1, NULL);
    mulle_thread_join(prod2, NULL);
    mulle_thread_join(cons, NULL);
    
    return 0;
}
```

## 6. Dependencies

- `mulle-thread`: Atomic operations for concurrent variant
- `mulle-c11`: Compatibility macros
