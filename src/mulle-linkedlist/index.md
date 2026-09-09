# mulle-linkedlist Library Documentation for AI
<!-- Keywords: lock-free, intrusive, linkedlist, concurrent, atomic, reclamation -->

## 1. Introduction & Purpose

`mulle-linkedlist` provides two intrusive, LIFO (head-insertion) linked lists
written in C:

- `struct _mulle_linkedlist` — a single-threaded variant.
- `struct _mulle_concurrent_linkedlist` — a lock-free variant whose head is held
  in a `mulle_atomic_pointer_t` and updated with CAS operations.

It is a "barebones" list: only `add` (prepend), `remove_one`, `remove_all`, and
`walk` are offered. There is no append, no lookup-by-key, no counting, no
allocation, and no free of entries — entries are intrusive and caller-owned.
It is used by `mulle-aba` and the `mulle-objc-runtime`, and is normally consumed
as part of `mulle-core` via CMake.

The problem it solves is safe, lock-free concurrent prepend/remove on a shared
intrusive list while keeping the memory model contract explicit and small enough
to reason about. The concurrent list does *not* perform memory reclamation;
callers must supply their own quiescence/ownership discipline.

## 2. Key Concepts & Design Philosophy

- **Intrusive entries.** The list stores only `_next` pointers. An entry struct is
  the *first* member of a caller-defined struct (so a pointer to the embedded
  entry can be cast back to the containing struct). The list never allocates or
  frees entries.
- **Single vs. concurrent.** The concurrent list is the real design target; the
  single-threaded list mimics its API so code can switch between the two with
  minimal churn.
- **Head as the only shared state.** All concurrent operations act on the single
  atomic head pointer. Entries are detached, made private to one thread, then
  re-published.
- **No reclamation.** Removing an entry does not make it safe to free or reuse
  while another thread may still hold a pointer to it. The caller owns the
  SMR/RCU/quiescence discipline.
- **Memory model.** The concurrent list uses exactly two `mulle-thread` orderings:
  default (seq_cst) for every CAS that publishes or consumes an entry, and
  `_relaxed` *only* for the speculative head load that feeds the CAS `expect`
  operand (never dereferenced). `_next` traversal of an already-detached chain
  and `init`/`done`/`walk` are non-atomic by contract. See
  `dox/MEMORY_MODEL.md` for the full rationale.
- **Lock-free, not wait-free.** All mutating operations are unbounded CAS retry
  loops.

## 3. Core API & Data Structures

All signatures are copied verbatim from the public headers
`src/mulle-linkedlist.h` and `src/mulle-concurrent-linkedlist.h`.

### 3.1. `mulle-linkedlist.h` — version, single-threaded list, entry helpers

#### Version
- `MULLE__LINKEDLIST_VERSION` is `((0UL << 20) | (1 << 8) | 0)` = 0.1.0.
- `static inline unsigned int   mulle_linkedlist_get_version_major( void)`
- `static inline unsigned int   mulle_linkedlist_get_version_minor( void)`
- `static inline unsigned int   mulle_linkedlist_get_version_patch( void)`
- `MULLE__LINKEDLIST_GLOBAL uint32_t   mulle_linkedlist_get_version( void);`

#### `struct _mulle_linkedlistentry`
- **Purpose:** The embedded link node for the single-threaded list; also usable
  by itself as a free-standing intrusive chain.
- **Key Fields:**
  - `struct _mulle_linkedlistentry   *_next;`
- **Entry chain helpers (operate on a bare `head` pointer, not on a list):**
  - `_mulle_linkedlistentry_chain( struct _mulle_linkedlistentry **head, struct _mulle_linkedlistentry *entry)` — prepend `entry` to the chain; O(1).
  - `_mulle_linkedlistentry_unchain( struct _mulle_linkedlistentry **head)` — pop the first entry (sets its `_next` to NULL); O(1), returns NULL if empty.
  - `typedef void  mulle_linkedlistentry_walk_callback_t( void *, struct _mulle_linkedlistentry *);`
  - `_mulle_linkedlistentry_walk( struct _mulle_linkedlistentry *next, mulle_linkedlistentry_walk_callback_t *callback, void *userinfo)` — walk a raw chain; "thinly disguised routine to call `mulle_allocator_free` on all entries."
  - `mulle_linkedlistentry_walk( struct _mulle_linkedlistentry *next, mulle_linkedlistentry_walk_callback_t *callback, void *userinfo)` — NULL-safe wrapper of the above.

#### `struct _mulle_linkedlist`
- **Purpose:** A single-threaded list (**NOT multi-threaded**). It is a
  single-node structure holding a head entry.
- **Key Fields:**
  - `struct _mulle_linkedlistentry   _head;`
- **Lifecycle Functions:**
  - `_mulle_linkedlist_init( struct _mulle_linkedlist *p)` — zero the list.
  - `_mulle_linkedlist_done( struct _mulle_linkedlist *p)` — asserts the list is empty.
- **Core Operations:**
  - `_mulle_linkedlist_add( struct _mulle_linkedlist *list, struct _mulle_linkedlistentry *entry)` — prepend `entry` (asserts entry `_next` is NULL and not already in list); O(1).
  - `_mulle_linkedlist_remove_one( struct _mulle_linkedlist *list)` — pop the first entry or return NULL; O(1).
  - `mulle_linkedlist_remove_one( struct _mulle_linkedlist *list)` — NULL-safe wrapper.
  - `_mulle_linkedlist_remove_all( struct _mulle_linkedlist *list)` — "retrieves the current head pointer and sets it to NULL in one atomic operation"; returns the detached chain (list remains usable).
- **Inspection / Callback Functions:**
  - `typedef int   mulle_linkedlist_walk_callback_t( struct _mulle_linkedlistentry *, struct _mulle_linkedlistentry *, void *);`
    Callback receives `(entry, prev, userinfo)`; return non-zero to stop.
  - `_mulle_linkedlist_walk( struct _mulle_linkedlist *list, mulle_linkedlist_walk_callback_t *callback, void *userinfo)` — walk; `next` is captured before the callback so the callback may unlink/free `entry`.
  - `mulle_linkedlist_walk( struct _mulle_linkedlist *list, mulle_linkedlist_walk_callback_t *callback, void *userinfo)` — NULL-safe wrapper.

### 3.2. `mulle-concurrent-linkedlist.h` — concurrent lock-free list

Included automatically by `mulle-linkedlist.h` unless
`MULLE_LINKEDLIST_NO_CONCURRENT` is defined.

#### `struct _mulle_concurrent_linkedlistentry`
- **Purpose:** The embedded link node for the concurrent list.
- **Key Fields:**
  - `struct _mulle_concurrent_linkedlistentry   *_next;`
- **Lifecycle note:** An entry must be unlinked and exclusively owned by the
  caller before it is added.

#### `union _mulle_concurrent_atomiclinkedlistentry`
- **Purpose:** Exposes the atomic head pointer for debugging via the `entry`
  member (never read it except in the debugger); the stored member is
  `mulle_atomic_pointer_t   pointer;`.

#### `struct _mulle_concurrent_linkedlist`
- **Purpose:** A lock-free list; concurrent `add`/`remove_one`/`remove_all` on
  the atomic head, no memory reclamation.
- **Key Fields:**
  - `union _mulle_concurrent_atomiclinkedlistentry   _head;`
- **Lifecycle Functions:**
  - `_mulle_concurrent_linkedlist_init( struct _mulle_concurrent_linkedlist *p)` — zero the list. Non-atomic by contract: the list must not yet be reachable by another thread.
  - `_mulle_concurrent_linkedlist_done( struct _mulle_concurrent_linkedlist *p)` — asserts the head is empty. Non-atomic by contract: all other threads must have quiesced.
- **Core Operations:**
  - `_mulle_concurrent_linkedlist_add( struct _mulle_concurrent_linkedlist *list, struct _mulle_concurrent_linkedlistentry  *entry)` — CAS-based prepend with retry; O(1) expected. The caller must finish all writes to `entry` *before* calling and must not touch it afterwards (publishing CAS is seq_cst).
  - `_mulle_concurrent_linkedlist_remove_all( struct _mulle_concurrent_linkedlist *list)` — atomically detach the whole chain (relaxed speculative read, seq_cst CAS); returns the chain, which is now private to the caller. Empty result means "empty at some point during this call".
  - `MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST struct _mulle_concurrent_linkedlistentry  * _mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list);` — remove one entry by detaching all, lopping off one entry, then republishing the remainder; worst-case O(n) in the retained chain length (implementation in `mulle-concurrent-linkedlist.c`).
  - `mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list)` — NULL-safe wrapper (static inline).
- **Callback / Walk Functions (NOT thread-safe):**
  - `MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST_SECOND int   _mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list, int (*callback)( struct _mulle_concurrent_linkedlistentry *, struct _mulle_concurrent_linkedlistentry *, void *), void *userinfo);` — reads `_head` non-atomically and traverses with plain loads; caller must have exclusive access.
  - `mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list, int (*callback)( struct _mulle_concurrent_linkedlistentry *, struct _mulle_concurrent_linkedlistentry *, void *), void *userinfo)` — NULL-safe wrapper (static inline).

## 4. Performance Characteristics

- **Single-threaded list:**
  - `_add` (prepend): O(1).
  - `_remove_one`: O(1).
  - `_remove_all`: O(1).
  - `_walk`: O(n), n = entries.
  - `chain`/`unchain`: O(1).
- **Concurrent list:**
  - `_add`: O(1) expected; unbounded CAS retry loop on contention.
  - `_remove_all`: O(1) expected; one CAS (stale relaxed reads just retry).
  - `_remove_one`: worst-case O(n) in the retained chain length — it detaches
    the whole chain, walks it to re-link the remainder, then republishes with a
    CAS. Avoid under high contention.
  - `_walk`: O(n); deliberately non-atomic, exclusive access required.
- **Memory:** Entries are intrusive and caller-allocated; the list itself adds no
  per-entry memory and performs no allocation. No memory reclamation is
  provided.
- **Thread-safety:** The concurrent list is lock-free (not wait-free) for
  `add`/`remove_all`/`remove_one`. `init`/`done`/`walk` and the single-threaded
  list require exclusive access / external locking.
- **Version constraints (enforced by `_mulle-linkedlist-versioncheck.h`):**
  `mulle-thread >= 4.10.0` (for the `_relaxed` atomic API), `mulle-c11 >= 4.9.0`,
  `mulle-allocator >= 8.1.0`.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:**
  - Embed the entry struct (e.g. `struct demo_entry` with `_link` as its first
    member) and cast between `&entry->_link` and `entry` with `(void *)`.
  - Complete all writes to an entry *before* `_add`, and release the entry only
    after quiescence — the list publishes/consumes via seq_cst CAS but performs
    no reclamation.
  - Use `_mulle_linkedlistentry_walk` with a free callback (it is designed as a
    drop-in wrapper around `mulle_allocator_free`) when tearing down a raw chain.
  - Call `mulle_linkedlist*_walk` only at a quiescent point, where the callback
    may safely consume/free `entry` (the walk reads `next` before invoking the
    callback).
  - For NULL-safe entry points, prefer the non-underscore wrappers
    (`mulle_linkedlist_remove_one`, `mulle_concurrent_linkedlist_remove_one`,
    `mulle_linkedlist_walk`, `mulle_concurrent_linkedlist_walk`,
    `mulle_linkedlistentry_walk`).
  - Follow the memory-model rules in `dox/MEMORY_MODEL.md`; do not introduce a
    plain/`_relaxed` `_write` on `_head` (that would cut the release sequence).
- **Common Pitfalls:**
  - Do not access `union _mulle_concurrent_atomiclinkedlistentry.entry` outside
    a debugger.
  - Do not free or reuse an entry after `remove_all`/`remove_one` while other
    threads may still hold a pointer to it — no reclamation is provided.
  - `_mulle_linkedlist_add` and `_mulle_concurrent_linkedlist_add` assert that
    `entry->_next == NULL`; re-adding a linked entry corrupts the list.
  - A NULL from `_mulle_concurrent_linkedlist_remove_all` means "empty at some
    point during this call", not "empty now".
  - The concurrent list only prepends; there is no append operation.
  - `_walk` on the concurrent list is **not thread-safe**.
- **Idiomatic Usage:**
  - Include `<mulle-linkedlist/mulle-linkedlist.h>` (the concurrent part comes
    along unless `MULLE_LINKEDLIST_NO_CONCURRENT` is defined); normally the list
    is reachable via the `mulle-core` umbrella header
    `#include <mulle-core/mulle-core.h>`.

## 6. Integration Examples

Code style: 3-space indent, Allman braces, one variable per line, `return( expr);`.

### Example 1: Single-threaded list — add, remove, walk

```c
#include <mulle-linkedlist/mulle-linkedlist.h>
#include <mulle-allocator/mulle-allocator.h>
#include <assert.h>


struct demo_entry
{
   struct _mulle_linkedlistentry   _link;
   void                            *_payload;
};


static int   free_callback( struct _mulle_linkedlistentry *entry,
                            struct _mulle_linkedlistentry *prev,
                            void *userinfo)
{
   mulle_allocator_free( (struct mulle_allocator *) userinfo,
                         entry);
   return( 0);
}


int   main( void)
{
   struct _mulle_linkedlist    list;
   struct demo_entry           *entry;
   unsigned int                i;

   _mulle_linkedlist_init( &list);

   for( i = 0; i < 4; i++)
   {
      entry = mulle_allocator_calloc( NULL, 1, sizeof( *entry));
      assert( entry);

      entry->_payload = (void *) (intptr_t) i;
      _mulle_linkedlist_add( &list, &entry->_link);       // prepend, O(1)
   }

   _mulle_linkedlist_walk( &list, free_callback, &mulle_allocator);

   assert( ! list._head._next);                           // list is drained, walk freed everything
   _mulle_linkedlist_done( &list);
   return( 0);
}
```

### Example 2: Concurrent list — init, publish, drain

```c
#include <mulle-linkedlist/mulle-linkedlist.h>
#include <mulle-allocator/mulle-allocator.h>
#include <assert.h>
#include <string.h>


struct demo_entry
{
   struct _mulle_concurrent_linkedlistentry   _link;
   unsigned long                              payload;   // written before add, read after remove
};

static struct _mulle_concurrent_linkedlist   list;


int   main( void)
{
   struct demo_entry   *entry;
   unsigned int        i;

   _mulle_concurrent_linkedlist_init( &list);

   for( i = 0; i < 4; i++)
   {
      entry = mulle_allocator_calloc( NULL, 1, sizeof( *entry));
      assert( entry);

      entry->payload = i;                                // finish ALL writes before add
      _mulle_concurrent_linkedlist_add( &list, &entry->_link);
   }

   // ... other threads may add/remove concurrently ...

   // quiescent point: only now is walking/draining safe
   while( (entry = (void *) _mulle_concurrent_linkedlist_remove_one( &list)))
   {
      assert( ! entry->_link._next);
      mulle_allocator_free( NULL, entry);                // safe once all producers/consumers quiesced
   }

   _mulle_concurrent_linkedlist_done( &list);
   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` dependencies (per `.mulle/etc/sourcetree/config` and
`clib.json`):

- `mulle-c11` — cross-platform C compiler glue; minimum 4.9.0.
- `mulle-allocator` — allocation scheme used for the free-on-walk convenience
  pattern; minimum 8.1.0.
- `mulle-thread` — provides `mulle_atomic_pointer_t` and the atomics/CAS API;
  minimum 4.10.0.

`mulle-linkedlist` is itself a component consumed by `mulle-core` (and used by
`mulle-aba` and the `mulle-objc-runtime`).