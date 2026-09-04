# mulle-linkedlist API summary
<!-- Keywords: lock-free, linked-list, intrusive, reclamation -->

`mulle-linkedlist` provides two intrusive LIFO linked lists:

- `struct _mulle_linkedlist` for single-threaded use.
- `struct _mulle_concurrent_linkedlist` for CAS-based concurrent head
  insertion/removal.

Entries are embedded in caller-owned structures. An entry must be unlinked and
owned exclusively by the caller before insertion.

## Single-threaded API

`struct _mulle_linkedlistentry` contains `_next`. The entry helpers are:

- `_mulle_linkedlistentry_chain(head, entry)` — prepend, O(1).
- `_mulle_linkedlistentry_unchain(head)` — remove the first entry, O(1).
- `mulle_linkedlistentry_walk(next, callback, userinfo)` — walk a chain.

`struct _mulle_linkedlist` provides `_init`, `_done`, `_add`,
`_remove_one`, `_remove_all`, and `mulle_linkedlist_walk` operations. It is not
thread-safe without external synchronization.

## Concurrent API

`struct _mulle_concurrent_linkedlistentry` contains `_next`; the list stores
its head in an atomic pointer. The operations are:

- `_mulle_concurrent_linkedlist_init` / `_done`.
- `_mulle_concurrent_linkedlist_add` — CAS-based prepend with retry.
- `_mulle_concurrent_linkedlist_remove_all` — atomically detach the chain.
- `_mulle_concurrent_linkedlist_remove_one` — remove one entry by rebuilding
  the remaining chain; worst-case O(n).
- `_mulle_concurrent_linkedlist_walk` — **not thread-safe**.

The concurrent list does not provide memory reclamation. Removing an entry does
not make it safe to free or reuse while another thread may still hold a pointer
to it. Callers must provide the required quiescence, ownership, or reclamation
discipline.

The list is lock-free, not wait-free: all mutating operations are unbounded CAS
retry loops. Publication uses the default sequentially consistent `mulle-thread`
operations; the relaxed form is used only for the speculative head read inside a
CAS retry loop. The full contract is in `dox/MEMORY_MODEL.md`. Requires
mulle-thread 4.9.0 or better.

The concurrent list prepends entries; it does not append them. The API does not
allocate or free entries.

See `dox/MEMORY_MODEL.md`, `dox/API_LINKEDLIST.md` and the source headers for
the contract and complete signatures.
