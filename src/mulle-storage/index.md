# mulle-storage Library Documentation for AI
<!-- Keywords: memory-pool, node-allocation -->

## 1. Introduction & Purpose

mulle-storage provides optimized memory management for allocating and freeing fixed-size elements (nodes) with automatic reuse of freed memory. It solves the problem of efficient node allocation in tree and data structure implementations where individual node allocation/deallocation would cause fragmentation and locality issues. The library offers two variants: `mulle_storage` (queue-based, maintains insertion order) and `mulle_indexedstorage` (index-based, direct access). This is a foundational component of the mulle-c ecosystem used extensively in tree and container implementations.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Node Pool Architecture:** Rather than allocating individual nodes with malloc/free, maintains a pool of pre-allocated fixed-size elements. Freed nodes are queued for reuse instead of being returned to the system.

- **Reduced Fragmentation:** By reusing freed slots internally, fragmentation is minimized. All nodes live in contiguous allocations, improving cache locality and reducing system allocator pressure.

- **Two Variants:**
  - `mulle_storage`: Elements accessed by pointer (FIFO queue of freed nodes for reuse).
  - `mulle_indexedstorage`: Elements accessed by integer indices (efficient for array-like access).

- **Fixed-Size Elements:** All elements in a storage must be the same size and alignment. Configured at initialization time.

- **Lazy Allocation:** Storage grows as needed, but capacity is pre-reserved to avoid repeated reallocation.

- **Fast Operations:** Allocation and deallocation are O(1) operations; no search or reorganization overhead.

- **Not Thread-Safe:** Requires external synchronization for multi-threaded access.

## 3. Core API & Data Structures

### 3.1 `mulle-storage.h` - Queue-Based Storage (Pointer Access)

#### `struct mulle_storage`

- **Purpose:** Container for fixed-size node allocation with queue-based freed node reuse.

- **Internal Structure:**
  - `_structs`: Underlying `mulle_structqueue` holding allocated nodes.
  - `_freed`: Pointer array tracking freed node addresses for reuse.

#### Lifecycle Functions

**`_mulle_storage_init(struct mulle_storage *alloc, size_t sizeof_struct, unsigned int alignof_struct, unsigned int capacity, struct mulle_allocator *allocator)`**

- **Purpose:** Initialize a storage container.
- **Parameters:**
  - `alloc`: Pointer to uninitialized `mulle_storage` structure.
  - `sizeof_struct`: Size of each element to be allocated.
  - `alignof_struct`: Alignment requirement of each element.
  - `capacity`: Initial capacity (number of elements to pre-allocate).
  - `allocator`: Memory allocator to use (NULL for default).
- **Behavior:** Pre-allocates memory for capacity elements.

**`mulle_storage_init(...)`** (NULL-safe wrapper)

- **Purpose:** Same as `_mulle_storage_init`, but safely handles NULL storage pointer.

**`_mulle_storage_done(struct mulle_storage *alloc)`**

- **Purpose:** Finalize a storage container and free all resources.
- **Parameters:**
  - `alloc`: Initialized storage pointer.
- **Behavior:** Frees all internal structures and resets the storage.

**`mulle_storage_done(...)`** (NULL-safe wrapper)

- **Purpose:** Same as `_mulle_storage_done`, but safely handles NULL pointer.

#### Allocation Functions

**`_mulle_storage_malloc(struct mulle_storage *alloc)`**

- **Purpose:** Allocate a node from the storage.
- **Returns:** Pointer to uninitialized node memory.
- **Behavior:**
  - Checks freed nodes queue first; reuses if available (O(1)).
  - Otherwise, reserves a new node from the queue (O(1)).

**`_mulle_storage_calloc(struct mulle_storage *alloc)`**

- **Purpose:** Allocate a node from storage and zero-initialize it.
- **Returns:** Pointer to zero-initialized node memory.
- **Behavior:** Calls `_mulle_storage_malloc()` then `memset()` to zero.

**`_mulle_storage_copy(struct mulle_storage *alloc, void *q)`**

- **Purpose:** Allocate a new node and copy data from an existing element.
- **Parameters:**
  - `alloc`: Storage container.
  - `q`: Source element to copy.
- **Returns:** Pointer to newly allocated and copied node.
- **Behavior:** Uses `_mulle_storage_malloc()` and `memcpy()`.

#### Deallocation Functions

**`_mulle_storage_free(struct mulle_storage *alloc, void *p)`**

- **Purpose:** Return a node to the storage for reuse.
- **Parameters:**
  - `alloc`: Storage container.
  - `p`: Pointer to node to free.
- **Behavior:**
  - Marks node as freed (debug fills with 0xDEADDEAD in debug mode).
  - Adds pointer to freed nodes queue for reuse.

#### Inspection Functions

**`_mulle_storage_get_allocator(struct mulle_storage *alloc)`**

- **Purpose:** Retrieve the allocator used by the storage.
- **Returns:** Pointer to the allocator.

**`mulle_storage_get_allocator(...)`** (NULL-safe)

- **Purpose:** Same as above, but safely handles NULL pointer (returns NULL).

**`_mulle_storage_get_count(struct mulle_storage *alloc)`**

- **Purpose:** Get the number of currently allocated (not freed) nodes.
- **Returns:** Count of active nodes (total reserved minus freed).

**`mulle_storage_get_count(...)`** (NULL-safe)

- **Purpose:** Same as above, but safely handles NULL pointer (returns 0).

**`_mulle_storage_get_element_size(struct mulle_storage *alloc)`**

- **Purpose:** Get the size of each element in the storage.
- **Returns:** Size in bytes.

### 3.2 `mulle-indexedstorage.h` - Index-Based Storage

#### `struct mulle_indexedstorage`

- **Purpose:** Fixed-size node storage with integer index-based access instead of pointers.

- **Internal Structure:**
  - `_structs`: Underlying `mulle_structarray` holding elements.
  - `_freed`: Pointer array tracking freed indices for reuse.

#### Key Differences from `mulle_storage`

- **Access Model:** Elements are accessed by unsigned integer indices rather than direct pointers.
- **Underlying Structure:** Uses `mulle_structarray` instead of `mulle_structqueue`.
- **Valid Pointer Lifetime:** Pointers are valid only until reallocation (not stable like queue-based storage).

#### Lifecycle Functions

**`_mulle_indexedstorage_init(struct mulle_indexedstorage *alloc, size_t sizeof_struct, unsigned int alignof_struct, unsigned int capacity, struct mulle_allocator *allocator)`**

- **Purpose:** Initialize indexed storage.
- **Parameters:** Same as `mulle_storage_init`.

**`_mulle_indexedstorage_done(struct mulle_indexedstorage *alloc)`**

- **Purpose:** Finalize and free indexed storage.

#### Allocation Functions

**`_mulle_indexedstorage_alloc(struct mulle_indexedstorage *alloc)`**

- **Purpose:** Allocate an index for a new element.
- **Returns:** Unsigned integer index (not a pointer).
- **Behavior:** Reuses freed indices or allocates new ones.

#### Access Functions

**`_mulle_indexedstorage_get(struct mulle_indexedstorage *alloc, unsigned int index)`**

- **Purpose:** Get a pointer to the element at the given index.
- **Parameters:**
  - `alloc`: Indexed storage container.
  - `index`: Element index.
- **Returns:** Pointer to the element (valid until next reallocation).
- **Warning:** Pointer is not stable; reallocation can invalidate it.

#### Deallocation Functions

**`_mulle_indexedstorage_free(struct mulle_indexedstorage *alloc, unsigned int index)`**

- **Purpose:** Return an index to the storage for reuse.
- **Parameters:**
  - `alloc`: Indexed storage container.
  - `index`: Index to free.

#### Inspection Functions

**`_mulle_indexedstorage_get_allocator(struct mulle_indexedstorage *alloc)`**

- **Purpose:** Retrieve the allocator used by indexed storage.

**`_mulle_indexedstorage_get_count(struct mulle_indexedstorage *alloc)`**

- **Purpose:** Get the number of currently allocated (not freed) elements.

**`_mulle_indexedstorage_get_element_size(struct mulle_indexedstorage *alloc)`**

- **Purpose:** Get the size of each element.

## 4. Performance Characteristics

- **Allocation Time:** O(1) amortized. Freed nodes are reused in FIFO order.
- **Deallocation Time:** O(1). Simply adds pointer to freed list.
- **Memory Overhead:** 16-32 bytes per node (internal structures) plus capacity overhead.
- **Fragmentation:** Minimal internal fragmentation; external fragmentation eliminated by reuse.
- **Cache Locality:** High; allocated nodes occupy contiguous memory regions.

**Comparisons:**

- **vs. malloc/free:** Much faster for repeated allocation/deallocation; better cache locality; higher initial memory cost.
- **vs. pre-allocated arrays:** Handles dynamic growth; reuses freed slots efficiently without holes.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Choose Correct Variant:**
   - Use `mulle_storage` for tree nodes accessed by pointer.
   - Use `mulle_indexedstorage` for graph nodes or when indices are natural.

2. **Estimate Capacity Correctly:** Initialize with reasonable capacity to minimize reallocations. Overcapacity is cheap; constant reallocations are expensive.

3. **Batch Operations:** Group allocations and deallocations to maximize freed list reuse benefits.

4. **Match Element Size & Alignment:** Ensure `sizeof_struct` and `alignof_struct` match actual structure requirements to avoid padding and alignment issues.

5. **Use Allocator Consistently:** Pass the same allocator to init that you'll use for the tree or data structure using the storage.

### Common Pitfalls:

1. **Double-Free:** Freeing the same pointer twice corrupts the freed list; use careful tracking.

2. **Freed Pointer Reuse (with `mulle_storage`):** After `_mulle_storage_free()`, the pointer may be reused; don't access the old pointer after freeing.

3. **Pointer Validity (with `mulle_indexedstorage`):** Pointers obtained from `_mulle_indexedstorage_get()` are invalidated on reallocation; cache indices, not pointers.

4. **Capacity Estimation:** Too small capacity causes frequent reallocations; too large wastes memory pre-allocated but never used.

5. **Alignment Mismatch:** Providing incorrect alignment can cause crashes on some architectures; always use `alignof()` macro.

### Idiomatic Usage:

```c
// Tree node storage
struct tree_node {
    struct tree_node *left, *right;
    int value;
};

struct mulle_storage node_store;
_mulle_storage_init(&node_store, 
                    sizeof(struct tree_node),
                    alignof(struct tree_node),
                    1000,      // initial capacity
                    NULL);     // default allocator

struct tree_node *new_node = _mulle_storage_malloc(&node_store);
// ... use node ...
_mulle_storage_free(&node_store, new_node);

_mulle_storage_done(&node_store);
```

## 6. Integration Examples

### Example 1: Basic Storage Usage

```c
#include <mulle-storage/mulle-storage.h>
#include <stdio.h>
#include <string.h>

struct point {
    int x;
    int y;
};

int main() {
    struct mulle_storage store;
    
    _mulle_storage_init(&store,
                        sizeof(struct point),
                        alignof(struct point),
                        10,
                        NULL);
    
    // Allocate some points
    struct point *p1 = _mulle_storage_malloc(&store);
    p1->x = 10;
    p1->y = 20;
    
    struct point *p2 = _mulle_storage_malloc(&store);
    p2->x = 30;
    p2->y = 40;
    
    printf("Points: (%d,%d) (%d,%d)\n", p1->x, p1->y, p2->x, p2->y);
    
    _mulle_storage_free(&store, p1);
    _mulle_storage_free(&store, p2);
    
    _mulle_storage_done(&store);
    
    return 0;
}
// Output: Points: (10,20) (30,40)
```

### Example 2: Storage with Reuse Verification

```c
#include <mulle-storage/mulle-storage.h>
#include <stdio.h>

struct data {
    int id;
    char name[32];
};

int main() {
    struct mulle_storage store;
    
    _mulle_storage_init(&store,
                        sizeof(struct data),
                        alignof(struct data),
                        5,
                        NULL);
    
    // Allocate first element
    struct data *d1 = _mulle_storage_malloc(&store);
    printf("Allocated d1 at %p\n", (void *)d1);
    
    // Allocate second element
    struct data *d2 = _mulle_storage_malloc(&store);
    printf("Allocated d2 at %p\n", (void *)d2);
    
    // Free first element
    _mulle_storage_free(&store, d1);
    
    // Allocate new element (should reuse d1's memory)
    struct data *d3 = _mulle_storage_malloc(&store);
    printf("Allocated d3 at %p (should be %p)\n", (void *)d3, (void *)d1);
    
    if (d3 == d1)
        printf("Memory reused successfully!\n");
    
    _mulle_storage_free(&store, d2);
    _mulle_storage_free(&store, d3);
    
    _mulle_storage_done(&store);
    
    return 0;
}
```

### Example 3: Copy Operation

```c
#include <mulle-storage/mulle-storage.h>
#include <stdio.h>
#include <string.h>

struct record {
    int id;
    double value;
    char label[16];
};

int main() {
    struct mulle_storage store;
    
    _mulle_storage_init(&store,
                        sizeof(struct record),
                        alignof(struct record),
                        20,
                        NULL);
    
    // Create a template record
    struct record template = {
        .id = 42,
        .value = 3.14,
    };
    strcpy(template.label, "template");
    
    // Copy into storage
    struct record *r1 = _mulle_storage_copy(&store, &template);
    struct record *r2 = _mulle_storage_copy(&store, &template);
    
    printf("Record 1: id=%d, value=%.2f, label=%s\n",
           r1->id, r1->value, r1->label);
    printf("Record 2: id=%d, value=%.2f, label=%s\n",
           r2->id, r2->value, r2->label);
    
    _mulle_storage_free(&store, r1);
    _mulle_storage_free(&store, r2);
    
    _mulle_storage_done(&store);
    
    return 0;
}
```

### Example 4: Tree Node Storage

```c
#include <mulle-storage/mulle-storage.h>
#include <stdlib.h>
#include <stdio.h>

struct tree_node {
    struct tree_node *left;
    struct tree_node *right;
    int value;
};

struct tree {
    struct tree_node *root;
    struct mulle_storage node_storage;
};

struct tree_node *create_node(struct tree *tree, int value) {
    struct tree_node *node = _mulle_storage_malloc(&tree->node_storage);
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    return node;
}

void delete_node(struct tree *tree, struct tree_node *node) {
    _mulle_storage_free(&tree->node_storage, node);
}

int main() {
    struct tree tree;
    tree.root = NULL;
    
    _mulle_storage_init(&tree.node_storage,
                        sizeof(struct tree_node),
                        alignof(struct tree_node),
                        100,
                        NULL);
    
    // Build simple tree
    tree.root = create_node(&tree, 10);
    tree.root->left = create_node(&tree, 5);
    tree.root->right = create_node(&tree, 15);
    
    printf("Tree created with root value: %d\n", tree.root->value);
    printf("Active nodes: %u\n", _mulle_storage_get_count(&tree.node_storage));
    
    // Clean up
    delete_node(&tree, tree.root->left);
    delete_node(&tree, tree.root->right);
    delete_node(&tree, tree.root);
    
    printf("After cleanup, active nodes: %u\n", _mulle_storage_get_count(&tree.node_storage));
    
    _mulle_storage_done(&tree.node_storage);
    
    return 0;
}
```

### Example 5: Indexed Storage Usage

```c
#include <mulle-storage/mulle-storage.h>
#include <stdio.h>

struct vertex {
    float x, y, z;
};

int main() {
    struct mulle_indexedstorage graph;
    
    _mulle_indexedstorage_init(&graph,
                               sizeof(struct vertex),
                               alignof(struct vertex),
                               1000,
                               NULL);
    
    // Allocate vertices by index
    unsigned int v1_idx = _mulle_indexedstorage_alloc(&graph);
    unsigned int v2_idx = _mulle_indexedstorage_alloc(&graph);
    
    struct vertex *v1 = _mulle_indexedstorage_get(&graph, v1_idx);
    v1->x = 1.0; v1->y = 0.0; v1->z = 0.0;
    
    struct vertex *v2 = _mulle_indexedstorage_get(&graph, v2_idx);
    v2->x = 0.0; v2->y = 1.0; v2->z = 0.0;
    
    printf("Vertex 1: (%.1f, %.1f, %.1f)\n", v1->x, v1->y, v1->z);
    printf("Vertex 2: (%.1f, %.1f, %.1f)\n", v2->x, v2->y, v2->z);
    
    _mulle_indexedstorage_free(&graph, v1_idx);
    _mulle_indexedstorage_free(&graph, v2_idx);
    
    _mulle_indexedstorage_done(&graph);
    
    return 0;
}
```

### Example 6: Large Batch Allocation and Reuse

```c
#include <mulle-storage/mulle-storage.h>
#include <stdio.h>

struct batch_item {
    int seq;
    int data;
};

int main() {
    struct mulle_storage store;
    
    _mulle_storage_init(&store,
                        sizeof(struct batch_item),
                        alignof(struct batch_item),
                        1000,
                        NULL);
    
    // Batch allocate
    struct batch_item *items[100];
    for (int i = 0; i < 100; i++) {
        items[i] = _mulle_storage_malloc(&store);
        items[i]->seq = i;
        items[i]->data = i * 10;
    }
    
    printf("Allocated 100 items, active count: %u\n",
           _mulle_storage_get_count(&store));
    
    // Free every third item
    for (int i = 0; i < 100; i += 3) {
        _mulle_storage_free(&store, items[i]);
    }
    
    printf("After freeing 33 items, active count: %u\n",
           _mulle_storage_get_count(&store));
    
    // Reallocate (will reuse freed slots)
    for (int i = 0; i < 33; i++) {
        items[i * 3] = _mulle_storage_malloc(&store);
        items[i * 3]->seq = 100 + i;
    }
    
    printf("After reallocation, active count: %u\n",
           _mulle_storage_get_count(&store));
    
    // Clean up
    for (int i = 0; i < 100; i++) {
        _mulle_storage_free(&store, items[i]);
    }
    
    _mulle_storage_done(&store);
    
    return 0;
}
```

## 7. Dependencies

Direct mulle-sde dependencies:
- `mulle-container`: Provides `mulle_structqueue`, `mulle_structarray`, and other container primitives used internally
