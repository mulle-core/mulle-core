<!-- keywords: sentinels, ownership, graphviz, fragmentation, monotonic, invariants -->

# Quirks

## The umbrella is not the behavior spec

- `src/mulle-core.h` is the export list plus version helpers. A top-level include that compiles does not mean you have found the owning semantics.
- Missing symbol bugs in this repo often reduce to "not re-exported in the envelope" rather than "implementation absent".

## Formatting bugs often look like memory bugs

- `src/mulle-buffer/mulle-buffer.h` says `mulle_buffer_do_string` blocks must not `return`; doing so leaks. Use the macro's extraction flow or `mulle_buffer_return`.
- The same header notes that `mulle_buffer_sprintf` does **not** append `'\0'`.
- `src/mulle-sprintf/mulle-sprintf.h` says `mulle_snprintf` always terminates but still signals overflow with `-1` and `errno = ENOMEM`.
- `mulle_asprintf` returns memory from the mulle allocator. Free it with `mulle_free`, not `free`.

## Container and tree helpers have sharp edges too

- `src/mulle-container/mulle-array.h` says you must not store `callback->notakey`. "Not found" is not always `NULL`.
- `src/mulle-container/mulle-container-callback-global.h` shows the common sentinels differ by callback: `mulle_not_an_int`, `mulle_not_an_intptr`, `NULL`, and `mulle_not_a_pointer` all appear.
- `src/mulle-container/mulle-map.h` exposes different duplicate policies in `insert`, `register`, `update`, and `set`. Do not debug them as if they were interchangeable writes.
- `src/mulle-container-debug/mulle-pointerarray-debug.h` is marked "use this only for debugging", and the concrete describe helper here is for pointer arrays.
- `src/mulle-rbtree-debug/mulle--rbtree-debug.c` returns a borrowed error string or `NULL` from `mulle__rbtree_validate`; do not free the result.

## Concurrent enumeration is intentionally fragile

- `src/mulle-concurrent/mulle-concurrent-hashmap.h` forbids `hash = 0` and `value = 0` or `INTPTR_MIN`.
- The same header labels `mulle_concurrent_hashmap_patch` experimental and less tested than the other operations.
- Its enumerator is only usable on the calling thread, and mutation or growth can stop it early with a mutation alert.
- `src/mulle-thread/mulle-thread.h` says `mulle_thread_mutex_do` must not be nested.
- `MULLE_THREAD_MUTEX_NEEDS_DONE` is `1`; cleanup is part of the contract, not an optional platform detail.

## Parser, UTF, and time failures are usually contract failures

- `src/mulle-http/TOC.md` says callback slices point into the current input buffer. Copy them if you need them after the callback returns.
- The same doc says header fields and values may arrive in many callbacks. One callback is not one logical field.
- `src/mulle-http/http_parser.h` wants `upgrade` checked alongside the `http_parser_execute` return value and `http_errno`.
- `src/mulle-utf/mulle-utf-rover.h` initializes rovers with `(pointer, len)` and advances against a sentinel, not a NUL terminator.
- `src/mulle-time/TOC.md` warns against mixing calendar and absolute time in arithmetic.
- `src/mulle-time/mulle-time.h` skips `mulle-timeval.h` on Windows, so cross-platform repro code should not assume it is present.
