# mulle-rbtree Library Documentation for AI
<!-- Keywords: red-black-tree, sorted -->

## 1. Introduction & Purpose

mulle-rbtree is a highly optimized red-black tree implementation for efficient sorted data storage and retrieval. It solves the problem of maintaining dynamically ordered collections with guaranteed O(log n) lookup, insertion, and deletion operations. This is a foundational data structure in the mulle-c ecosystem, used for implementing sorted containers and indices. The library provides both a low-level API (`mulle__rbtree`) for managing raw nodes and a higher-level convenience API (`mulle_rbtree`) that handles memory management through callbacks.

## 2. Key Concepts & Design Philosophy

The red-black tree is a self-balancing binary search tree where nodes are colored either red or black. This coloring guarantees that the tree remains relatively balanced after every insertion and deletion, preventing worst-case O(n) performance. 

**Key Design Principles:**

- **Dual API Levels:** The library offers a low-level `mulle__rbtree` API for maximum control (direct node management) and a convenient `mulle_rbtree` API for ease of use (payload-based operations with automatic memory management via callbacks).

- **Allocator Integration:** Uses `mulle_allocator` for all memory operations, allowing custom allocation strategies and tracking.

- **Value Callbacks:** The high-level API uses `mulle_container_valuecallback` structures to manage lifecycle (retain/release) of stored values, similar to reference counting patterns.

- **Comparison Function:** Values are sorted using a user-provided comparison function returning `<0` (less), `0` (equal), or `>0` (greater).

- **Extra Node Data:** Supports storing custom data alongside tree nodes via "extra" bytes, enabling derived data structures like segment trees or weighted trees.

- **Dirty Tracking:** Nodes can be marked dirty to enable deferred processing or change tracking.

- **Not Thread-Safe:** Tree operations require external synchronization; no built-in thread safety mechanisms.

## 3. Core API & Data Structures

### 3.1 `mulle-rbtree.h` - High-Level Convenience API

#### `struct mulle_rbtree`

- **Purpose:** Opaque convenient wrapper around the red-black tree that manages payload lifecycle automatically.

- **Key Fields:** (Private, implementation details; accessed via functions)
  - Internal `mulle__rbtree` base structure
  - Comparison function pointer for sorting payloads
  - Value callback structure for retain/release operations
  - Optional "dirty" callback for tracking changes

- **Lifecycle Functions:**
  - `mulle_rbtree_init()` / `_mulle_rbtree_init()`: Initialize a tree with a comparison function, value callbacks, and allocator.
  - `mulle_rbtree_init_with_config()` / `_mulle_rbtree_init_with_config()`: Initialize with a config structure for advanced options.
  - `mulle_rbtree_done()` / `_mulle_rbtree_done()`: Clean up the tree, releasing all payloads via the callback.

- **Core Operations:**
  - `mulle_rbtree_add()` / `_mulle_rbtree_add()`: Insert a value into the tree (returns 0 on success, errno on failure).
  - `mulle_rbtree_remove()` / `_mulle_rbtree_remove()`: Remove a value from the tree (returns 0 on success).
  - `mulle_rbtree_remove_node()` / `_mulle_rbtree_remove_node()`: Remove a specific node by its address.
  - `mulle_rbtree_find()`: Locate a value by key (exact match via comparison function).
  - `mulle_rbtree_find_equal_or_greater()`: Find a value matching the key or the next greater one.
  - `mulle_rbtree_walk()`: Iterate forward through values in sorted order, calling a callback for each.

- **Enumeration:**
  - `mulle_rbtree_enumerate()`: Create an enumerator for forward iteration.
  - `mulle_rbtreeenumerator_next()`: Get the next value from an enumerator.
  - `mulle_rbtree_for()`: Macro for convenient forward iteration loops.
  - `mulle_rbtree_reverseenumerate()`: Create an enumerator for reverse iteration.
  - `mulle_rbtreereverseenumerator_next()`: Get the next value from a reverse enumerator.
  - `mulle_rbtree_reversefor()`: Macro for convenient reverse iteration loops.

- **Inspection:**
  - `mulle_rbtree_get_allocator()`: Retrieve the allocator used by the tree.
  - `_mulle_rbtree_walk_dirty()`: Process all nodes marked as dirty.

#### `struct mulle_rbtree_config`

- **Purpose:** Configuration structure for advanced tree initialization.

- **Fields:**
  - `comparison`: Function pointer for comparing payloads.
  - `dirty`: Optional callback invoked when nodes are marked dirty.
  - `callback`: Pointer to value callback structure for lifecycle management.
  - `node_extra`: Number of extra bytes to allocate per node (for custom data).
  - `options`: Bitfield for initialization options (reserved for future use).

### 3.2 `mulle--rbtree.h` - Low-Level Implementation API

#### `struct mulle__rbtree`

- **Purpose:** The underlying red-black tree structure managing tree topology and operations at the node level.

- **Usage Pattern:** Direct node manipulation; used by library developers and advanced users needing fine-grained control.

- **Lifecycle Functions:**
  - `_mulle__rbtree_init()`: Initialize an empty tree with options and allocator.
  - `_mulle__rbtree_done()`: Destroy the tree and free all nodes.

- **Node Operations:**
  - `_mulle__rbtree_new_node()`: Create a new tree node with a payload.
  - `_mulle__rbtree_free_node()`: Free a node (the node must be already removed from the tree).
  - `_mulle__rbtree_insert_node()`: Insert a node into the tree using a comparison function (returns 0 on success).
  - `_mulle__rbtree_insert_node_before_node()`: Insert a new node directly before an existing node (bypasses comparison).
  - `_mulle__rbtree_insert_node_after_node()`: Insert a new node directly after an existing node (bypasses comparison).
  - `_mulle__rbtree_remove_node()`: Remove a node from the tree.
  - `_mulle__rbtree_get_node_value()`: Extract the payload from a node (works with extra data).

- **Tree Navigation:**
  - `_mulle__rbtree_find_node()`: Locate a node by payload using a comparison function.
  - `_mulle__rbtree_find_node_equal_or_greater()`: Find a node with matching payload or the next greater one.
  - `_mulle__rbtree_next_node()`: Get the in-order successor of a node.
  - `_mulle__rbtree_previous_node()`: Get the in-order predecessor of a node.
  - `_mulle__rbtree_find_leftmost_node()`: Find the smallest element in a subtree.
  - `_mulle__rbtree_find_rightmost_node()`: Find the largest element in a subtree.

- **Iteration:**
  - `_mulle__rbtree_walk()`: Call a callback for each node in sorted order.
  - `_mulle__rbtree_walk_reverse()`: Call a callback for each node in reverse sorted order.

- **Dirty Tracking:**
  - `_mulle__rbtree_mark_node_as_dirty()`: Mark a node as dirty.
  - `_mulle__rbtree_walk_dirty()`: Process nodes marked as dirty.

- **Inspection:**
  - `_mulle__rbtree_get_allocator()`: Retrieve the allocator.
  - `_mulle__rbtree_get_root_node()`: Get the root node.
  - `_mulle__rbtree_get_nil_node()`: Get the sentinel nil node.
  - `mulle__rbtree_validate()`: Validate tree invariants (returns 0 if valid).

#### `struct mulle_rbnode`

- **Purpose:** Individual tree node structure containing tree topology and payload storage.

- **Key Fields:**
  - `_parent`: Pointer to parent node (managed internally).
  - `_left`: Pointer to left child (managed internally).
  - `_right`: Pointer to right child (managed internally).
  - `_color`: Node color (red/black) and dirty flag (managed internally).
  - `payload`: Void pointer to the stored value.

- **Node Utilities:**
  - `_mulle_rbnode_get_payload()`: Retrieve the payload from a node.
  - `_mulle_rbnode_set_payload()`: Set a node's payload (no retain semantics).
  - `_mulle_rbnode_get_extra()`: Access custom extra data appended to the node.
  - `_mulle_rbnode_is_black()`: Check if a node is black.
  - `_mulle_rbnode_is_red()`: Check if a node is red.
  - `_mulle_rbnode_is_dirty()`: Check if a node is marked dirty.
  - `_mulle_rbnode_set_dirty()`: Mark a node as dirty.
  - `_mulle_rbnode_clear_dirty()`: Clear the dirty flag.

### 3.3 Comparison Function Convention

All tree operations that search or insert require a comparison function with this prototype:

```c
int (*comparison)(void *a, void *b)
```

**Return value interpretation:**
- `< 0`: `a < b` (a comes before b in sort order)
- `0`: `a == b` (values are equal)
- `> 0`: `a > b` (a comes after b in sort order)

**Standard comparisons:**
- `strcmp` for C strings
- Pointer comparison for custom structures (with custom comparator function)

## 4. Performance Characteristics

- **Insertion:** O(log n) average and worst-case time, O(1) space per node.
- **Deletion:** O(log n) average and worst-case time.
- **Search (find/lookup):** O(log n) average and worst-case time.
- **Walk/Enumerate:** O(n) time to visit all nodes.
- **Memory:** Each node requires 48 bytes on 64-bit systems (5 pointers + color field) plus payload space and any extra data.

**Trade-offs:**
- **Space vs. Complexity:** Red-black trees use more memory than simpler structures (arrays) but guarantee logarithmic operations and support dynamic insertion/deletion.
- **Rebalancing Cost:** Insertions and deletions may require tree rotations and color changes, adding constant-factor overhead compared to simpler trees.

**Thread Safety:** The tree is not thread-safe. External synchronization (mutexes, atomic operations) is required for multi-threaded access.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Use the High-Level API When Possible:** `mulle_rbtree` is simpler and safer than `mulle__rbtree`; it automatically manages payloads via callbacks.

2. **Provide Correct Comparison Functions:** The comparison function is critical; incorrect implementations lead to corrupt trees or logic errors. Test comparison functions thoroughly, especially with edge cases (NULL values, equal elements, etc.).

3. **Always Call Done/Cleanup:** Ensure `mulle_rbtree_done()` is called to release payloads and internal memory. Use RAII patterns if available.

4. **Choose Appropriate Value Callbacks:** Use `mulle_container_valuecallback_copied_cstring` for string payloads or provide custom callbacks that correctly retain/release your data type.

5. **Allocator Management:** Pass the correct allocator; mismatch between allocation and deallocation can cause corruption.

### Common Pitfalls:

1. **Modifying Payloads In-Place:** Do not modify payload values after insertion without removing and re-adding the node; this breaks tree sort order invariants.

2. **Comparison Function Side Effects:** Comparison functions should be pure (no side effects); they may be called multiple times for a single operation.

3. **Memory Leaks with Custom Callbacks:** If using custom value callbacks, ensure retain() and release() are correctly paired; missing releases leak memory.

4. **Iterating While Modifying:** Do not remove or add nodes while walking/enumerating; use separate passes or collect nodes first, then modify.

5. **NULL Tree Operations:** Some functions check for NULL tree pointers and return silently; always validate tree pointers before use in performance-critical code.

### Idiomatic Usage:

```c
// Simple iteration over sorted values
mulle_rbtree_for(tree, value)
{
    process_value(value);
}

// Reverse iteration
mulle_rbtree_reversefor(tree, value)
{
    process_value_reverse(value);
}

// Manual enumeration with early exit
struct mulle_rbtreeenumerator rover = mulle_rbtree_enumerate(tree);
void *value;
while (_mulle_rbtreeenumerator_next(&rover, &value))
{
    if (should_break(value))
        break;
}
mulle_rbtreeenumerator_done(&rover);
```

## 6. Integration Examples

### Example 1: Basic String Tree Creation and Iteration

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>
#include <string.h>

struct mulle_rbtree tree;

// Initialize with strcmp for sorting strings,
// copied_cstring callback to retain/release strings
mulle_rbtree_init(&tree,
                   (int (*)(void *, void *)) strcmp,
                   &mulle_container_valuecallback_copied_cstring,
                   NULL);  // Use default allocator

// Add values
mulle_rbtree_add(&tree, "apple");
mulle_rbtree_add(&tree, "cherry");
mulle_rbtree_add(&tree, "banana");

// Iterate in sorted order and print
mulle_rbtree_for(&tree, value)
{
    printf("%s\n", (char *) value);
}
// Output: apple, banana, cherry

mulle_rbtree_done(&tree);
```

### Example 2: Finding and Removing Values

```c
#include <mulle-rbtree/mulle-rbtree.h>

struct mulle_rbtree tree;
mulle_rbtree_init(&tree,
                   (int (*)(void *, void *)) strcmp,
                   &mulle_container_valuecallback_copied_cstring,
                   NULL);

mulle_rbtree_add(&tree, "alice");
mulle_rbtree_add(&tree, "bob");
mulle_rbtree_add(&tree, "charlie");

// Find exact match
char *found = mulle_rbtree_find(&tree, "bob");
if (found)
    printf("Found: %s\n", found);  // Output: Found: bob

// Find equal or greater (useful for range queries)
char *greater = mulle_rbtree_find_equal_or_greater(&tree, "anna");
if (greater)
    printf("Next: %s\n", greater);  // Output: Next: bob

// Remove value
int result = mulle_rbtree_remove(&tree, "bob");
if (result == 0)
    printf("Removed bob\n");

mulle_rbtree_done(&tree);
```

### Example 3: Reverse Iteration and Walking with Callback

```c
#include <mulle-rbtree/mulle-rbtree.h>

static int print_callback(void *value, void *userinfo)
{
    printf("%s\n", (char *) value);
    return 1;  // Continue iteration
}

struct mulle_rbtree tree;
mulle_rbtree_init(&tree,
                   (int (*)(void *, void *)) strcmp,
                   &mulle_container_valuecallback_copied_cstring,
                   NULL);

mulle_rbtree_add(&tree, "dog");
mulle_rbtree_add(&tree, "cat");
mulle_rbtree_add(&tree, "ant");
mulle_rbtree_add(&tree, "bear");

// Walk forward
printf("Forward:\n");
mulle_rbtree_walk(&tree, print_callback, NULL);

// Reverse iteration
printf("Reverse:\n");
mulle_rbtree_reversefor(&tree, value)
{
    printf("%s\n", (char *) value);
}

mulle_rbtree_done(&tree);
```

### Example 4: Custom Comparison Function (Integers)

```c
#include <mulle-rbtree/mulle-rbtree.h>

// Custom comparison for integer pointers
static int compare_ints(void *a, void *b)
{
    int *ia = (int *) a;
    int *ib = (int *) b;
    return (*ia < *ib) ? -1 : (*ia > *ib) ? 1 : 0;
}

// Custom callback to free integers on release
static void int_release(struct mulle_container_valuecallback *callback,
                        void *p,
                        struct mulle_allocator *allocator)
{
    mulle_free(p);
}

static void *int_retain(struct mulle_container_valuecallback *callback,
                        void *p,
                        struct mulle_allocator *allocator)
{
    int *copy = mulle_malloc(sizeof(int), allocator);
    if (copy)
        *copy = *(int *) p;
    return copy;
}

struct mulle_rbtree tree;
struct mulle_container_valuecallback int_callback = {
    .release = int_release,
    .retain = int_retain
};

mulle_rbtree_init(&tree, compare_ints, &int_callback, NULL);

int val1 = 42, val2 = 10, val3 = 99;
mulle_rbtree_add(&tree, &val1);
mulle_rbtree_add(&tree, &val2);
mulle_rbtree_add(&tree, &val3);

// Find will locate values in sorted order (10, 42, 99)
int search_key = 42;
int *found = mulle_rbtree_find(&tree, &search_key);
if (found)
    printf("Found: %d\n", *found);

mulle_rbtree_done(&tree);
```

### Example 5: Low-Level Node API Usage

```c
#include <mulle-rbtree/mulle--rbtree.h>
#include <stdlib.h>
#include <string.h>

struct mulle__rbtree tree;
_mulle__rbtree_init(&tree, NULL);

// Create and insert nodes directly
struct mulle_rbnode *node1 = _mulle__rbtree_new_node(&tree, strdup("zebra"));
struct mulle_rbnode *node2 = _mulle__rbtree_new_node(&tree, strdup("apple"));
struct mulle_rbnode *node3 = _mulle__rbtree_new_node(&tree, strdup("mango"));

_mulle__rbtree_insert_node(&tree, node1, (void *) strcmp);
_mulle__rbtree_insert_node(&tree, node2, (void *) strcmp);
_mulle__rbtree_insert_node(&tree, node3, (void *) strcmp);

// Manually walk and print nodes
struct mulle_rbnode *nil = _mulle__rbtree_get_nil_node(&tree);
struct mulle_rbnode *current = _mulle__rbtree_find_leftmost_node(&tree,
                                                                 _mulle__rbtree_get_root_node(&tree));

while (current != nil)
{
    char *payload = _mulle_rbnode_get_payload(current);
    printf("%s\n", payload);
    current = _mulle__rbtree_next_node(&tree, current);
}

// Clean up
_mulle__rbtree_done(&tree);
```

### Example 6: Enumerator with Early Exit

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <string.h>

struct mulle_rbtree tree;
mulle_rbtree_init(&tree,
                   (int (*)(void *, void *)) strcmp,
                   &mulle_container_valuecallback_copied_cstring,
                   NULL);

mulle_rbtree_add(&tree, "one");
mulle_rbtree_add(&tree, "two");
mulle_rbtree_add(&tree, "three");
mulle_rbtree_add(&tree, "four");

// Enumerate with early exit condition
struct mulle_rbtreeenumerator rover = mulle_rbtree_enumerate(&tree);
void *value;

while (_mulle_rbtreeenumerator_next(&rover, &value))
{
    printf("Processing: %s\n", (char *) value);
    if (!strcmp((char *) value, "three"))
    {
        printf("Found target, stopping\n");
        break;
    }
}

mulle_rbtreeenumerator_done(&rover);
mulle_rbtree_done(&tree);
```

## 7. Dependencies

Direct mulle-sde dependencies:
- `mulle-storage`: Memory management and allocation utilities for tree nodes and internal structures
