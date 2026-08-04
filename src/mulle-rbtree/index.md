# mulle-rbtree Library Documentation for AI
<!-- Keywords: red-black-tree, sorted -->

## 1. Introduction & Purpose

mulle-rbtree is a red-black tree implementation that organizes data in a self-balancing binary search tree structure. It provides O(log n) insertion, deletion, and search operations while maintaining balanced tree properties. The library uses [mulle-allocator](https://github.com/mulle-c/mulle-allocator) to simplify memory management and is not thread-safe. It is a component of the [mulle-core](https://github.com/mulle-core/mulle-core) library.

Key features include:
- Self-balancing binary search tree with guaranteed O(log n) operations
- Two API levels: low-level node manipulation (`mulle__rbtree`) and high-level value-based operations (`mulle_rbtree`)
- Support for custom comparison functions and value callbacks for memory management
- Optional "extra" memory allocation within nodes for embedded data structures
- Forward and reverse enumeration support
- Dirty tracking mechanism for change detection
- Marker support for node tagging

## 2. Key Concepts & Design Philosophy

### Red-Black Tree Properties
mulle-rbtree implements a classic red-black tree as described in "Introduction to Algorithms" by Cormen, Leiserson, and Rivest (MIT Press, 1990, ISBN 0-07-013143-0). The tree maintains the following properties:
1. Every node is either red or black
2. The root is black
3. All nil (leaf) nodes are black
4. Red nodes have only black children
5. All paths from root to leaves contain the same number of black nodes

### Two-Level API Design
The library provides two distinct APIs that serve different use cases:

**`mulle__rbtree` (Low-Level API):**
- Direct node manipulation
- Requires manual node creation and payload management
- Uses `struct mulle_rbnode` explicitly
- More control but more responsibility for memory management
- Function prefix: `_mulle__rbtree_` (double underscore indicates unsafe/low-level)

**`mulle_rbtree` (High-Level API):**
- Value-based operations (hides node structure)
- Automatic node creation and management
- Uses `struct mulle_container_valuecallback` for automatic memory management
- Simpler interface with integrated retain/release semantics
- Function prefix: `_mulle_rbtree_` (single underscore after namespace)

### Payload vs. Extra Memory Modes
The tree can operate in two modes:

1. **Payload Pointer Mode (default):** The node stores a pointer to external data. Comparison function receives these pointers. Useful when you already have allocated objects.

2. **Extra Memory Mode (`mulle_rbtree_option_use_extra`):** Additional memory is allocated after each node structure. The data is embedded directly in the node. Comparison function receives pointers to the extra memory area. Useful for value types or when you want to avoid separate allocations.

### Dirty Tracking and Markers
- **Dirty Flag:** Nodes can be marked as "dirty" when their position changes due to tree balancing. The `_mulle__rbtree_walk_dirty` function allows you to process all dirty nodes once, useful for maintaining external indexes or caches.
- **Marker Flag:** Nodes can be marked for application-specific purposes (e.g., garbage collection marking).

### Nil Node Sentinel
The tree uses a sentinel "nil" node instead of NULL pointers for leaves. This simplifies boundary checking in tree operations and is a standard red-black tree implementation technique.

## 3. Core API & Data Structures

### 3.1. `mulle--rbtree.h` (Low-Level API)

#### Color Constants
```c
enum
{
   mulle__rbtree_black  = 0,
   mulle__rbtree_red    = 1,
   mulle__rbtree_marker = 0x40,
   mulle__rbtree_dirty  = 0x80
};
```

#### Option Flags
```c
enum
{
   mulle_rbtree_option_allow_duplicates = 0x1,
   mulle_rbtree_option_use_extra        = 0x2,
   mulle_rbtree_option_use_dirty        = 0x4,
   mulle_rbtree_option_use_marker       = 0x8
};
```

#### `struct mulle_rbnode`
The fundamental node structure for the red-black tree.

**Purpose:** Represents a single node in the tree with payload and tree structure pointers.

**Key Fields:**
- `_parent`: Pointer to parent node
- `_left`: Pointer to left child
- `_right`: Pointer to right child  
- `_color`: Node color (red/black) and flags (dirty, marker)
- `payload`: Pointer to user data (or start of extra memory area)

**Lifecycle Functions:**
- Nodes are created via `_mulle__rbtree_new_node()` 
- Nodes are freed automatically by `_mulle__rbtree_remove_node()` or `_mulle__rbtree_done()`

**Accessor Functions:**
- `_mulle_rbnode_get_payload()`: Get payload pointer
- `_mulle_rbnode_set_payload()`: Set payload pointer (does not retain)
- `_mulle_rbnode_get_extra()`: Get pointer to extra memory area (when using extra mode)
- `_mulle_rb_get_node_from_extra()`: Reverse mapping from extra pointer to node

**Color and Flag Functions:**
- `_mulle_rbnode_get_color()`: Get node color (black or red)
- `_mulle_rbnode_is_black()`: Check if node is black
- `_mulle_rbnode_is_red()`: Check if node is red
- `_mulle_rbnode_set_color()`: Set node color
- `_mulle_rbnode_set_red()`: Set node to red
- `_mulle_rbnode_set_black()`: Set node to black
- `_mulle_rbnode_is_dirty()`: Check if node is marked dirty
- `_mulle_rbnode_set_dirty()`: Mark node as dirty
- `_mulle_rbnode_clear_dirty()`: Clear dirty flag
- `_mulle_rbnode_is_marked()`: Check if node is marked
- `_mulle_rbnode_set_marker()`: Mark node
- `_mulle_rbnode_clear_marker()`: Clear marker

**Tree Navigation:**
- `_mulle_rbnode_get_parent()`: Get parent node
- `_mulle_rbnode_get_grandparent()`: Get grandparent node

#### `struct mulle__rbtree`
The low-level tree structure.

**Purpose:** Container for the red-black tree structure with direct node access.

**Key Fields:**
- `_root`: Pointer to root node
- `_nil`: Sentinel nil node (replaces NULL)
- `_nodes`: Storage allocator for nodes (struct mulle_storage)
- `_options`: Configuration flags

**Lifecycle Functions:**
- `_mulle__rbtree_init()`: Initialize tree with allocator
- `_mulle__rbtree_init_with_options()`: Initialize with extra memory size and options
- `_mulle__rbtree_done()`: Destroy tree structure (does not release payloads)

**Node Creation:**
- `_mulle__rbtree_new_node()`: Allocate and initialize new node with payload
- `_mulle__rbtree_free_node()`: Free a node (typically not called directly)

**Core Operations:**
- `_mulle__rbtree_insert_node()`: Insert node into tree using comparison function; returns -1 if duplicate
- `_mulle__rbtree_insert_node_before_node()`: Insert node before a specific node
- `_mulle__rbtree_insert_node_after_node()`: Insert node after a specific node  
- `_mulle__rbtree_remove_node()`: Remove and free node from tree

**Search Functions:**
- `_mulle__rbtree_find_node()`: Find exact match or return nil node
- `_mulle__rbtree_find_node_equal_or_greater()`: Find exact or next greater node
- `_mulle__rbtree_find_node_with_payload()`: Find node by payload (non-extra mode)
- `_mulle__rbtree_find_node_with_extra()`: Find node by extra memory (extra mode)
- `_mulle__rbtree_find_leftmost_node()`: Find leftmost node in subtree
- `_mulle__rbtree_find_rightmost_node()`: Find rightmost node in subtree

**Navigation:**
- `_mulle__rbtree_next_node()`: Get next node in sorted order (returns nil node at end)
- `_mulle__rbtree_previous_node()`: Get previous node in sorted order (returns nil node at start)

**Traversal:**
- `_mulle__rbtree_walk()`: Walk tree in sorted order with callback
- `_mulle__rbtree_walk_reverse()`: Walk tree in reverse sorted order with callback
- `_mulle__rbtree_walk_dirty()`: Process all dirty nodes and clear flags

**Inspection:**
- `_mulle__rbtree_get_root_node()`: Get root node
- `_mulle__rbtree_is_root_node()`: Check if node is root
- `_mulle__rbtree_get_nil_node()`: Get sentinel nil node
- `_mulle__rbtree_is_nil_node()`: Check if node is nil sentinel
- `_mulle__rbtree_get_allocator()`: Get allocator
- `_mulle__rbtree_get_count()`: Get number of nodes in tree (recursive count)
- `_mulle__rbtree_is_dirty()`: Check if tree has dirty nodes
- `_mulle__rbtree_get_extra_size()`: Get size of extra memory per node

**Dirty and Marker Support:**
- `_mulle__rbtree_mark_node_as_dirty()`: Mark node and propagate dirty flag to ancestors
- `_mulle__rbtree_set_node_marked()`: Mark node if marker option enabled
- `_mulle__rbtree_enable_marking()`: Enable marker support

**Helper Functions:**
- `_mulle__rbtree_get_node_value()`: Get value (payload or extra) from node
- `_mulle__rbtree_get_node_from_extra()`: Get node from extra memory pointer
- `_mulle__rbtree_init_node()`: Initialize node structure

### 3.2. `mulle-rbtree.h` (High-Level API)

#### `struct mulle_rbtree`
The high-level, value-based tree structure.

**Purpose:** User-friendly tree interface that hides node management and provides automatic value lifecycle management via callbacks.

**Key Fields:**
- Inherits all fields from `mulle__rbtree` via `MULLE__RBTREE_BASE` macro
- `comparison`: Function pointer for value comparison
- `dirty`: Optional callback for dirty node notifications
- `callback`: Value callback structure for retain/release/copy operations

**Lifecycle Functions:**
- `mulle_rbtree_init()`: Initialize tree with comparison function, value callback, and allocator (NULL-safe)
- `_mulle_rbtree_init()`: Unsafe version (does not check for NULL tree)
- `mulle_rbtree_init_with_config()`: Initialize with configuration structure (NULL-safe)
- `_mulle_rbtree_init_with_config()`: Unsafe version
- `mulle_rbtree_done()`: Destroy tree and release all values via callback (NULL-safe)
- `_mulle_rbtree_done()`: Unsafe version

#### `struct mulle_rbtree_config`
Configuration structure for advanced initialization.

**Purpose:** Group all initialization parameters for complex tree configurations.

**Fields:**
- `comparison`: Comparison function pointer
- `dirty`: Dirty notification callback (optional)
- `callback`: Pointer to value callback structure
- `node_extra`: Size of extra memory per node (for extra mode)
- `options`: Option flags (allow_duplicates, use_extra, use_dirty, use_marker)

**Core Operations:**
- `mulle_rbtree_add()`: Add value to tree; returns 0 on success, errno on failure (NULL-safe)
- `_mulle_rbtree_add()`: Unsafe version; handles retain via callback
- `mulle_rbtree_remove()`: Remove value from tree; returns 0 on success, errno on failure (NULL-safe)
- `_mulle_rbtree_remove()`: Unsafe version; handles release via callback
- `mulle_rbtree_remove_node()`: Remove by node pointer (NULL-safe)
- `_mulle_rbtree_remove_node()`: Unsafe version

**Search Functions:**
- `mulle_rbtree_find()`: Find value in tree; returns value pointer or NULL
- `mulle_rbtree_find_equal_or_greater()`: Find exact match or next greater value

**Traversal:**
- `mulle_rbtree_walk()`: Walk tree with value callback (NULL-safe for tree)
- `_mulle_rbtree_walk_dirty()`: Process dirty nodes with dirty callback

**Inspection:**
- `mulle_rbtree_get_allocator()`: Get allocator (NULL-safe)
- `_mulle_rbtree_get_allocator()`: Unsafe version

#### `struct mulle_rbtreeenumerator`
Forward enumerator for tree values.

**Purpose:** Iterate through tree values in sorted order.

**Key Fields:**
- `_tree`: Pointer to tree structure
- `_node`: Current node pointer

**Lifecycle Functions:**
- `mulle_rbtree_enumerate()`: Create enumerator starting at leftmost node
- `_mulle_rbtreeenumerator_next()`: Advance to next value; returns 1 if value available, 0 at end
- `_mulle_rbtreeenumerator_done()`: Cleanup enumerator (currently no-op)
- `mulle_rbtreeenumerator_done()`: NULL-safe cleanup

**Convenience Macro:**
- `mulle_rbtree_for( tree, item)`: Foreach-style iteration over values

#### `struct mulle_rbtreereverseenumerator`
Reverse enumerator for tree values.

**Purpose:** Iterate through tree values in reverse sorted order.

**Key Fields:**
- `_tree`: Pointer to tree structure  
- `_node`: Current node pointer

**Lifecycle Functions:**
- `mulle_rbtree_reverseenumerate()`: Create enumerator starting at rightmost node
- `_mulle_rbtreereverseenumerator_next()`: Advance to previous value; returns 1 if value available, 0 at end
- `_mulle_rbtreereverseenumerator_done()`: Cleanup enumerator (currently no-op)
- `mulle_rbtreereverseenumerator_done()`: NULL-safe cleanup

**Convenience Macro:**
- `mulle_rbtree_reversefor( tree, item)`: Foreach-style reverse iteration over values

## 4. Performance Characteristics

### Time Complexity
- **Insert:** O(log n) - guaranteed by red-black balancing
- **Delete:** O(log n) - guaranteed by red-black balancing
- **Search:** O(log n) - binary search on balanced tree
- **Find Min/Max:** O(log n) - walk to leftmost/rightmost node
- **Successor/Predecessor:** O(log n) amortized, O(1) for most cases
- **Enumeration:** O(n) - visit each node once

### Space Complexity
- **Per Node:** `sizeof(struct mulle_rbnode)` + `node_extra` bytes
  - Minimum: 5 pointers + 1 int ≈ 44 bytes on 64-bit systems (with payload)
  - Additional memory for extra mode as configured
- **Tree Overhead:** 1 sentinel node + storage structure ≈ 60 bytes
- **Balanced Height:** Maximum height is 2 * log₂(n + 1), typically very shallow

### Trade-offs
- **Red-Black vs. AVL:** Red-black trees are less strictly balanced than AVL trees, leading to slightly slower searches but significantly faster insertions and deletions (fewer rotations).
- **Extra Mode:** Using extra mode saves one pointer indirection on access but requires copying data into the node.
- **Dirty Tracking:** When enabled, adds overhead of flag propagation during insertions/deletions but allows efficient change notification.

### Thread-Safety
**Not thread-safe.** The library does not provide any internal synchronization. External locking is required for concurrent access. Multiple readers require external synchronization due to potential tree modifications affecting traversal.

### Memory Management
- Uses `mulle_storage` internally for efficient node allocation and bulk deallocation
- Storage grows dynamically but does not shrink until tree destruction
- Value lifecycle managed via callbacks in high-level API

## 5. AI Usage Recommendations & Patterns

### Best Practices

**1. Choose the Right API Level:**
- Use `mulle_rbtree` for most cases - it's safer and handles memory automatically
- Use `mulle__rbtree` only when you need fine control over nodes or are building another abstraction

**2. Always Initialize Properly:**
```c
struct mulle_rbtree tree;
mulle_rbtree_init( &tree, comparison_func, &callback, allocator);
// ... use tree ...
mulle_rbtree_done( &tree);  // Always clean up
```

**3. Use Value Callbacks Correctly:**
- `mulle_container_valuecallback_nonowned_cstring` - for string literals or managed strings
- `mulle_container_valuecallback_copied_cstring` - for strings that need duplication
- Custom callbacks for objects that need retain/release

**4. Comparison Function Requirements:**
- Must return negative if a < b, zero if a == b, positive if a > b
- Must be consistent and transitive
- The first argument is always the search key

**5. Handle NULL Safely:**
- Use the non-underscore variants for NULL-safe operations
- Use underscore-prefixed variants in tight loops where NULL is impossible

**6. Use Enumerators for Iteration:**
```c
void *value;
mulle_rbtree_for( &tree, value)
{
   // Process value
}
```

**7. Extra Mode Usage:**
- Set `node_extra` to size of your data structure minus sizeof(void *)
- Comparison receives pointer to extra area, not payload pointer
- Use `_mulle__rbtree_get_node_from_extra()` to convert extra pointer back to node if needed

### Common Pitfalls

**1. Forgetting to Call _done():**
```c
// BAD: Memory leak
struct mulle_rbtree tree;
mulle_rbtree_init( &tree, ...);
// ... use tree ...
// Forgot: mulle_rbtree_done( &tree);
```

**2. Using Wrong Callback:**
```c
// BAD: String literals with copied callback - wastes memory
mulle_rbtree_init( &tree, strcmp, 
                   &mulle_container_valuecallback_copied_cstring, NULL);
mulle_rbtree_add( &tree, "literal");  // Unnecessary copy

// GOOD: Use nonowned for literals
mulle_rbtree_init( &tree, strcmp,
                   &mulle_container_valuecallback_nonowned_cstring, NULL);
```

**3. Modifying Tree During Enumeration:**
```c
// BAD: Undefined behavior
mulle_rbtree_for( &tree, value)
{
   mulle_rbtree_remove( &tree, value);  // Corrupts enumeration!
}

// GOOD: Collect items first, then remove
```

**4. Direct Field Access on Private Fields:**
```c
// BAD: Accessing private fields
size_t len = tree._nodes._length;  // Private field!

// GOOD: Use accessor functions
size_t count = _mulle__rbtree_get_count( (struct mulle__rbtree *) &tree);
```

**5. Comparing Pointers Instead of Values:**
```c
// BAD: Comparing pointer addresses
int bad_compare( void *a, void *b)
{
   return( a - b);  // Compares addresses, not string content!
}

// GOOD: Compare actual content
int good_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}
```

**6. Forgetting Node Ownership After Remove:**
```c
// BAD: Using node after _mulle__rbtree_remove_node
_mulle__rbtree_remove_node( &tree, node);
void *value = _mulle_rbnode_get_payload( node);  // Use after free!

// GOOD: Get value before removing
void *value = _mulle_rbnode_get_payload( node);
_mulle__rbtree_remove_node( &tree, node);
```

### Idiomatic Usage

**1. Sorted String Set:**
```c
struct mulle_rbtree tree;
mulle_rbtree_init( &tree, (int (*)( void *, void *)) strcmp,
                   &mulle_container_valuecallback_copied_cstring, NULL);
mulle_rbtree_add( &tree, "banana");
mulle_rbtree_add( &tree, "apple");
// Iterate in sorted order
char *str;
mulle_rbtree_for( &tree, str)
   printf( "%s\n", str);  // Prints: apple, banana
mulle_rbtree_done( &tree);
```

**2. Using Extra Mode for Embedded Structures:**
```c
struct my_data
{
   int key;
   char name[32];
};

int compare_data( void *a, void *b)
{
   struct my_data *da = a, *db = b;
   return( da->key - db->key);
}

struct mulle_rbtree_config config =
{
   .comparison = compare_data,
   .callback   = NULL,
   .node_extra = sizeof( struct my_data) - sizeof( void *),
   .options    = mulle_rbtree_option_use_extra
};

struct mulle_rbtree tree;
mulle_rbtree_init_with_config( &tree, &config, NULL);

struct my_data item = { .key = 42, .name = "Answer" };
mulle_rbtree_add( &tree, &item);  // Copied into node
```

**3. Low-Level Node Manipulation:**
```c
struct mulle__rbtree tree;
_mulle__rbtree_init( &tree, NULL);

struct mulle_rbnode *node1 = _mulle__rbtree_new_node( &tree, "hello");
struct mulle_rbnode *node2 = _mulle__rbtree_new_node( &tree, "world");

_mulle__rbtree_insert_node( &tree, node1, (int (*)( void *, void *)) strcmp);
_mulle__rbtree_insert_node( &tree, node2, (int (*)( void *, void *)) strcmp);

// Walk manually
struct mulle_rbnode *node;
for( node = _mulle__rbtree_find_leftmost_node( &tree, _mulle__rbtree_get_root_node( &tree));
     node != _mulle__rbtree_get_nil_node( &tree);
     node = _mulle__rbtree_next_node( &tree, node))
{
   printf( "%s\n", (char *) _mulle_rbnode_get_payload( node));
}

_mulle__rbtree_done( &tree);
```

## 6. Integration Examples

### Example 1: Creating and Populating a Tree with String Values

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>
#include <string.h>

int   main( void)
{
   struct mulle_rbtree   tree;
   char                  *fruits[] = { "banana", "apple", "cherry", "date" };
   char                  *fruit;
   int                   i;

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_copied_cstring,
                      NULL);

   for( i = 0; i < 4; i++)
      mulle_rbtree_add( &tree, fruits[ i]);

   printf( "Sorted fruits:\n");
   mulle_rbtree_for( &tree, fruit)
   {
      printf( "  %s\n", fruit);
   }

   mulle_rbtree_done( &tree);
   return( 0);
}
```

### Example 2: Finding and Removing Values

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>
#include <string.h>

int   main( void)
{
   struct mulle_rbtree   tree;
   char                  *found;

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);

   mulle_rbtree_add( &tree, "red");
   mulle_rbtree_add( &tree, "green");
   mulle_rbtree_add( &tree, "blue");

   found = mulle_rbtree_find( &tree, "green");
   if( found)
      printf( "Found: %s\n", found);

   if( mulle_rbtree_remove( &tree, "green") == 0)
      printf( "Removed green\n");

   found = mulle_rbtree_find( &tree, "green");
   if( ! found)
      printf( "green no longer in tree\n");

   mulle_rbtree_done( &tree);
   return( 0);
}
```

### Example 3: Using Extra Mode for Value Types

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>

struct entry
{
   int      id;
   double   value;
};

static int   compare_entries( void *a, void *b)
{
   struct entry   *ea;
   struct entry   *eb;

   ea = (struct entry *) a;
   eb = (struct entry *) b;
   return( ea->id - eb->id);
}

int   main( void)
{
   struct mulle_rbtree          tree;
   struct mulle_rbtree_config   config;
   struct entry                 entries[] = { {3, 3.14}, {1, 1.41}, {2, 2.71} };
   struct entry                 *found;
   struct entry                 search_key;
   int                          i;

   config.comparison = compare_entries;
   config.dirty      = NULL;
   config.callback   = NULL;
   config.node_extra = sizeof( struct entry) - sizeof( void *);
   config.options    = mulle_rbtree_option_use_extra;

   mulle_rbtree_init_with_config( &tree, &config, NULL);

   for( i = 0; i < 3; i++)
      mulle_rbtree_add( &tree, &entries[ i]);

   printf( "Entries in sorted order:\n");
   mulle_rbtree_for( &tree, found)
   {
      printf( "  ID: %d, Value: %.2f\n", found->id, found->value);
   }

   search_key.id = 2;
   found         = mulle_rbtree_find( &tree, &search_key);
   if( found)
      printf( "Found entry with ID 2: value = %.2f\n", found->value);

   mulle_rbtree_done( &tree);
   return( 0);
}
```

### Example 4: Low-Level API with Node Manipulation

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int   callback( struct mulle_rbnode *node, void *userinfo)
{
   char   *payload;

   payload = _mulle_rbnode_get_payload( node);
   printf( "%s\n", payload);
   mulle_free( payload);
   return( 1);
}

int   main( void)
{
   struct mulle__rbtree    tree;
   struct mulle_rbnode     *node;
   char                    *words[] = { "zebra", "alpha", "beta" };
   int                     i;

   _mulle__rbtree_init( &tree, NULL);

   for( i = 0; i < 3; i++)
   {
      node = _mulle__rbtree_new_node( &tree, mulle_strdup( words[ i]));
      if( _mulle__rbtree_insert_node( &tree, node, (int (*)( void *, void *)) strcmp) != 0)
      {
         fprintf( stderr, "Failed to insert %s\n", words[ i]);
         mulle_free( _mulle_rbnode_get_payload( node));
         _mulle__rbtree_free_node( &tree, node);
      }
   }

   printf( "Sorted words:\n");
   _mulle__rbtree_walk( &tree, callback, NULL);

   _mulle__rbtree_done( &tree);
   return( 0);
}
```

### Example 5: Reverse Enumeration

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>

static int   compare_ints( void *a, void *b)
{
   return( (int)(intptr_t) a - (int)(intptr_t) b);
}

int   main( void)
{
   struct mulle_rbtree   tree;
   void                  *value;
   int                   i;

   mulle_rbtree_init( &tree,
                      compare_ints,
                      &mulle_container_valuecallback_nonowned_pointer_or_null,
                      NULL);

   for( i = 1; i <= 5; i++)
      mulle_rbtree_add( &tree, (void *)(intptr_t) i);

   printf( "Forward: ");
   mulle_rbtree_for( &tree, value)
   {
      printf( "%d ", (int)(intptr_t) value);
   }
   printf( "\n");

   printf( "Reverse: ");
   mulle_rbtree_reversefor( &tree, value)
   {
      printf( "%d ", (int)(intptr_t) value);
   }
   printf( "\n");

   mulle_rbtree_done( &tree);
   return( 0);
}
```

### Example 6: Using Find Equal or Greater

```c
#include <mulle-rbtree/mulle-rbtree.h>
#include <stdio.h>

static int   compare_ints( void *a, void *b)
{
   return( (int)(intptr_t) a - (int)(intptr_t) b);
}

int   main( void)
{
   struct mulle_rbtree   tree;
   void                  *found;
   int                   values[] = { 10, 20, 30, 40, 50 };
   int                   i;

   mulle_rbtree_init( &tree,
                      compare_ints,
                      &mulle_container_valuecallback_nonowned_pointer_or_null,
                      NULL);

   for( i = 0; i < 5; i++)
      mulle_rbtree_add( &tree, (void *)(intptr_t) values[ i]);

   found = mulle_rbtree_find_equal_or_greater( &tree, (void *)(intptr_t) 25);
   if( found)
      printf( "Equal or greater to 25: %d\n", (int)(intptr_t) found);

   found = mulle_rbtree_find_equal_or_greater( &tree, (void *)(intptr_t) 30);
   if( found)
      printf( "Equal or greater to 30: %d\n", (int)(intptr_t) found);

   found = mulle_rbtree_find_equal_or_greater( &tree, (void *)(intptr_t) 60);
   if( ! found)
      printf( "No value >= 60\n");

   mulle_rbtree_done( &tree);
   return( 0);
}
```

## 7. Dependencies

mulle-rbtree has the following direct dependency:
- **mulle-storage** - Provides efficient block allocation for tree nodes

Through mulle-storage, it transitively depends on:
- **mulle-container** - Provides value callback structures
- **mulle-allocator** - Memory allocation abstraction layer  
- **mulle-data** - Data structure utilities
- **mulle-c11** - C11 compatibility and portability layer

These dependencies establish mulle-rbtree as a mid-level component in the mulle-c library hierarchy, suitable for building higher-level container abstractions.
