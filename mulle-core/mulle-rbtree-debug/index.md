# mulle-rbtree-debug Library Documentation for AI
<!-- Keywords: red-black-tree, validation, debugging, graphviz, dot, ascii, visualization -->

## 1. Introduction & Purpose

mulle-rbtree-debug is a small debugging companion library for
[mulle-rbtree](https://github.com/mulle-c/mulle-rbtree). It adds two kinds of
facilities that do not belong inside the production tree code itself:

1. **Integrity validation** (`mulle__rbtree_validate`): checks that a
   `struct mulle__rbtree` still satisfies all five red-black tree properties
   and returns a descriptive error-message string when it does not.
2. **Visualization** (`mulle__rbtree_node_dot_fprintf`,
   `mulle__rbtree_node_ascii_fprintf`): renders the internal topology of a
   tree as Graphviz DOT source or as ASCII art, including node color and
   "dirty" state.

It is used by the `mulle-rbtree` test suite to verify that insert/delete
operations never corrupt tree invariants. It resides in `mulle-core` because
it builds on `mulle-fprintf` for all its I/O. It is a debug/utility library
only; it contains no tree-mutating logic.

## 2. Key Concepts & Design Philosophy

The library never alters a tree. It only *reads* the internal state of a
`struct mulle__rbtree` — its `_root` node, the sentinel `nil` node, the node
`_left`/`_right`/`_parent` pointers, and the `_color`/dirty flags — then either
checks invariants or renders the topology.

- **Target type is `struct mulle__rbtree`** (defined in `mulle-rbtree`), read
  through the `mulle__rbtree_*` / `mulle_rbnode*` accessor helpers of that
  library (`_mulle__rbtree_get_nil_node`, `_mulle__rbtree_get_root_node`,
  `_mulle__rbtree_get_node_value`, `_mulle_rbnode_is_red`,
  `_mulle_rbnode_is_black`, `_mulle_rbnode_is_dirty`,
  `_mulle_rbnode_is_marked`, ...). The debug functions expect a fully
  initialized tree.
- **Validation checks the five classic red-black properties**: (1) every node
  is red or black, (2) the root is black, (3) the NIL leaves are black, (4) a
  red node never has a red child, (5) every root-to-leaf path contains the
  same number of black nodes. It additionally checks parent-child pointer
  consistency and that a marked-dirty node always has a marked-dirty parent.
- **Validation reports an error string, not a bool.** `mulle__rbtree_validate`
  returns `NULL` when the tree is valid and a human-readable constant
  `char *` (e.g. `"Root node is not black"`) for the first violation found.
  The returned string must not be freed.
- **Value printing is pluggable.** The DOT printer takes a
  `void (*)(FILE *, void *)` callback and falls back to printing the numeric
  node id when the callback is `NULL`. The ASCII printer takes a
  `char *(*)(void *)` callback that must return a freshly allocated,
  `mulle_free`-able string, and it **requires** a non-`NULL` callback (the
  implementation calls `abort()` otherwise).
- **I/O goes through `mulle-fprintf`.** A `NULL` `fp` routes output to
  `stdout`.
- **Not thread-safe.** Like the underlying tree, these helpers require
  external synchronization when shared.

## 3. Core API & Data Structures

The complete public API lives in a single header, `src/mulle-rbtree-debug.h`
(which includes the generated `mulle-rbtree-debug/include.h`). There are no
other public headers and no data structures of its own — it only operates on
the `struct mulle__rbtree` / `struct mulle_rbnode` types supplied by
`mulle-rbtree`. All exported functions are declared with the
`MULLE__RBTREE__DEBUG_GLOBAL` linkage macro.

### 3.1 `mulle-rbtree-debug.h`

#### `mulle__rbtree_validate`

- **Purpose:** Verify that a red-black tree satisfies all red-black tree
  invariants (root/NIL black, no red-red, equal black heights, consistent
  parent pointers, consistent dirty flags).
- **Signature (verbatim):**

```c
MULLE__RBTREE__DEBUG_GLOBAL
char  *mulle__rbtree_validate(struct mulle__rbtree *a_tree);
```

- **Return value:**
  - `NULL` — the tree is valid. Also returned when `a_tree == NULL` or the
    tree is empty (root == nil).
  - otherwise — a constant error-message string describing the first
    violation (do not free it).

#### `mulle__rbtree_node_dot_fprintf`

- **Purpose:** Write the tree as Graphviz DOT source (a `digraph RBTree {...}`
  block) for use with `dot`/`xdot` or for debugging dumps.
- **Signature (verbatim):**

```c
MULLE__RBTREE__DEBUG_GLOBAL
void  mulle__rbtree_node_dot_fprintf( FILE *fp,
                                      struct mulle__rbtree *tree,
                                      void (*print_value_fn)( FILE *fp, void *));
```

- **Behavior:**
  - `fp == NULL` → output goes to `stdout`.
  - `print_value_fn == NULL` → node labels are the numeric node id (printed
    with `mulle_fprintf`).
  - Present children get edges labeled `L`/`R`; missing children are drawn as
    point-shaped `nil_<id>` leaves.
  - Fill color encodes state: `red`/`black` normally, `lightcoral`/`darkgray`
    when dirty. If the tree was created with the
    `mulle_rbtree_option_use_marker` option, unmarked nodes use `dotted` or
    `dashed` styles instead.

#### `mulle__rbtree_node_ascii_fprintf`

- **Purpose:** Render the tree as a human-readable ASCII diagram with `\`/`/`
  connector lines.
- **Signature (verbatim):**

```c
MULLE__RBTREE__DEBUG_GLOBAL
void  mulle__rbtree_node_ascii_fprintf( FILE *fp,
                                        struct mulle__rbtree *tree,
                                        char *(*print_value_fn)( void *));
```

- **Behavior:**
  - `fp == NULL` → output goes to `stdout`.
  - `print_value_fn` **must not be `NULL`** (the implementation calls
    `abort()` otherwise). It is called once per node and must return a
    `char *`; the library frees it with `mulle_free`.
  - An empty tree prints `(empty tree)`; a `NULL` tree prints `NULL`.
  - Each node label is the printed value followed by a state character:
    `r` red, `b` black, `R` red+dirty, `B` black+dirty.

#### Version macro

```c
#define MULLE__RBTREE__DEBUG_VERSION   ((0UL << 20) | (1 << 8) | 3)
```

Consumed by the mulle-sde dependency version-check mechanism. Bumped from
`(1 << 8) | 2` to `(1 << 8) | 3`.

## 4. Performance Characteristics

- **Validation:** O(n) over the visited nodes. It recurses into every subtree
  (red-red/parent/dirty checks) and makes an additional pass per node while
  computing black heights. It allocates no heap memory; stack usage is O(tree
  height), worst case O(n) for a degenerate input.
- **DOT output:** O(n) — one or two short `mulle_fprintf` lines per node plus
  per-child edges, written incrementally.
- **ASCII output:** O(n) for the layout pass, then O(n × (n + line width))
  string assembly in the current implementation (parent nodes are located by
  linear scan and level buffers are copied). Fine for small debug trees;
  avoid for large ones.
- **Memory:** DOT output allocates nothing; ASCII output allocates one label
  string per node (freed internally) and one line buffer per tree level.
- **Thread safety:** Not thread-safe; requires external synchronization.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:**
  - Add the dependency with `mulle-sde add github:mulle-core/mulle-rbtree-debug`
    and include `<mulle-rbtree-debug/mulle-rbtree-debug.h>`.
  - Call `mulle__rbtree_validate` after any sequence of inserts/deletes in
    tests; treat any non-`NULL` return as a hard error and report the string.
  - Pass `NULL` as the `print_value_fn` for a minimal DOT dump (numeric ids)
    or provide a callback when payloads are wanted.
  - Always use the ASCII printer with a callback that `mulle_malloc`s its
    result string — the library frees it.
  - Guard debug output behind a debug flag/switch in production code; this
    library adds I/O overhead and is meant for diagnostics.

- **Common Pitfalls:**
  - `mulle__rbtree_validate` returns a `char *`, not an `int`. Check for
    `NULL` (valid) vs. non-`NULL` (invalid) — do not test `== 0`.
  - Do **not** free the error string returned by `mulle__rbtree_validate`; it
    is a constant, shared message.
  - Do **not** pass a `NULL` `print_value_fn` to
    `mulle__rbtree_node_ascii_fprintf` — it aborts.
  - The printing callbacks must not retain or deep-copy nothing except what
    they print; DOT uses the value pointer as-is.
  - These helpers only diagnose/render; they never fix a broken tree.

- **Idiomatic usage (from the test suite):**
  - The validation kept in tests: build a tree, run mutating operations, then
    `err = mulle__rbtree_validate( &tree); if( err) fail if err != NULL`.
  - The dependency's `mulle__rbtree` API is the low-level node API
    (`_mulle__rbtree_init`, `_mulle_storage_malloc`/`_mulle__rbtree_init_node`
    or `_mulle__rbtree_new_node`, `_mulle__rbtree_remove_node`,
    `_mulle__rbtree_done`).

## 6. Integration Examples

The examples below build small trees by hand (as the test suite does, using
the low-level `mulle-rbtree` node API) so they are self-contained and only
depend on the two public debug functions.

### Example 1: Validating a Manually Built Tree

```c
#include <mulle-rbtree-debug/mulle-rbtree-debug.h>
#include <stdio.h>

int  main( void)
{
   struct mulle__rbtree   tree;
   struct mulle_rbnode    *node1;
   struct mulle_rbnode    *node2;
   struct mulle_rbnode    *node3;
   char                   *err;

   _mulle__rbtree_init( &tree, NULL);

   // root node, black
   node1 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node1, (void *) 5);
   node1->_color = mulle__rbtree_black;

   // two red children
   node2 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node2, (void *) 3);
   node2->_color  = mulle__rbtree_red;
   node2->_parent = node1;
   node1->_left   = node2;

   node3 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node3, (void *) 7);
   node3->_color  = mulle__rbtree_red;
   node3->_parent = node1;
   node1->_right  = node3;

   tree._root = node1;

   // NULL means valid, otherwise err is a constant message string
   err = mulle__rbtree_validate( &tree);
   if( err)
   {
      fprintf( stderr, "INVALID: %s\n", err);
      return( 1);
   }

   _mulle__rbtree_done( &tree);
   return( 0);
}
```

### Example 2: Dumping a Tree as Graphviz DOT

```c
#include <mulle-rbtree-debug/mulle-rbtree-debug.h>
#include <stdio.h>

static void  print_name( FILE *fp, void *value)
{
   fprintf( fp, "%s", (char *) value);
}

int  main( void)
{
   struct mulle__rbtree   tree;
   struct mulle_rbnode    *node1;
   struct mulle_rbnode    *node2;
   struct mulle_rbnode    *node3;

   _mulle__rbtree_init( &tree, NULL);

   node1 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node1, (void *) "root");
   node1->_color = mulle__rbtree_black;

   node2 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node2, (void *) "left");
   node2->_color  = mulle__rbtree_red;
   node2->_parent = node1;
   node1->_left   = node2;

   node3 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node3, (void *) "right");
   node3->_color  = mulle__rbtree_red;
   node3->_parent = node1;
   node1->_right  = node3;

   tree._root = node1;

   mulle__rbtree_node_dot_fprintf( stdout, &tree, print_name);

   _mulle__rbtree_done( &tree);
   return( 0);
}
```

### Example 3: Rendering a Tree as ASCII Art

```c
#include <mulle-rbtree-debug/mulle-rbtree-debug.h>
#include <stdio.h>

static char  *print_int( void *value)
{
   char   *s;

   s = mulle_malloc( 16);
   sprintf( s, "%d", (int)(intptr_t) value);
   return( s);
}

int  main( void)
{
   struct mulle__rbtree   tree;
   struct mulle_rbnode    *node1;
   struct mulle_rbnode    *node2;
   struct mulle_rbnode    *node3;

   _mulle__rbtree_init( &tree, NULL);

   node1 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node1, (void *) 5);
   node1->_color = mulle__rbtree_black;

   node2 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node2, (void *) 3);
   node2->_color  = mulle__rbtree_red;
   node2->_parent = node1;
   node1->_left   = node2;

   node3 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node3, (void *) 7);
   node3->_color  = mulle__rbtree_red;
   node3->_parent = node1;
   node1->_right  = node3;

   tree._root = node1;

   mulle__rbtree_node_ascii_fprintf( stdout, &tree, print_int);

   _mulle__rbtree_done( &tree);
   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-rbtree` — provides `struct mulle__rbtree`, `struct mulle_rbnode`,
  and every node/tree accessor the debug functions operate on.
- `mulle-fprintf` — used for all formatted output (`mulle_fprintf`).

## 8. Shortcut

The previous `index.md` was committed as `b5b10d1` ("docs: add comprehensive
AI-oriented API documentation") on 2026-08-04. Since that commit, the only
public-API change is a version bump (`MULLE__RBTREE__DEBUG_VERSION` from
`(1 << 8) | 2` to `(1 << 8) | 3`). The prior document mistakenly described the
`mulle-rbtree` dependency API instead of this project's API, so it was
rewritten here to document the actual three public functions of
`mulle-rbtree-debug`.