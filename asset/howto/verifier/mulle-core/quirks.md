<!-- keywords: sentinels, ownership, fragmentation, monotonic, sparse-tests, cleanup -->

# Quirks

## The repo is an envelope first

- `README.md` and `mulle-core/mulle-core.h` describe an amalgamation, not one dominant runtime subsystem.
- A failure in this repo is often "not re-exported by the envelope anymore" before it is "implementation broken".
- `test/README.md` is a generic harness description. Do not assume there is already a focused local regression test for the family you touched.

## Buffer and sprintf checks have ownership traps

- `mulle-core/mulle-buffer/mulle-buffer.h` says `mulle_buffer_do_string` blocks must not `return`; doing so leaks.
- `mulle-core/mulle-sprintf/mulle-sprintf.h` says `mulle_buffer_sprintf` does **not** append `'\0'`.
- The same header says `mulle_snprintf` always terminates but still reports overflow as `-1` with `errno = ENOMEM`.
- `mulle_asprintf` returns mulle-allocator memory. Verifying with plain `free` hides the real contract.

## Container tests must honor the callback contract

- `mulle-core/mulle-container/mulle-array.h` says you must not store `callback->notakey`; "not found" is not always `NULL`.
- `mulle-core/mulle-container/mulle-container-callback-global.h` shows the common callback presets use different sentinels (`NULL`, `mulle_not_an_int`, `mulle_not_an_intptr`, `mulle_not_a_pointer`).
- `mulle-core/mulle-container/mulle-map.h` exposes four distinct write paths: `insert`, `register`, `update`, and `set`. A verifier test that treats them as interchangeable is wrong.
- The same array header allows integer-via-pointer callbacks, but it also says `mulle-structarray` is the better dense-integer fit. Do not mistake "possible" for "preferred".

## Concurrent helpers are strict, not forgiving

- `mulle-core/mulle-concurrent/mulle-concurrent-hashmap.h` forbids `hash = 0` and `value = 0` or `INTPTR_MIN`.
- The same header labels `mulle_concurrent_hashmap_patch` experimental and less tested than the other operations.
- Its enumerator is only valid on the calling thread, and mutation or growth can stop iteration early with `ECANCELLED`.
- `mulle-core/mulle-thread/mulle-thread.h` says `mulle_thread_mutex_do` must not be nested.
- `MULLE_THREAD_MUTEX_NEEDS_DONE` is `1`; verifier code must not skip cleanup on the theory that the platform will do it anyway.

## Parser, UTF, and time coverage is mostly contract coverage

- `mulle-core/mulle-http/http_parser.h` says `http_data_cb` may be called arbitrarily many times for one logical string.
- `mulle-core/mulle-http/TOC.md` says callback slices point into the current input buffer. Copy them if you need them after callback return.
- Always compare `http_parser_execute` against input length and inspect `parser.http_errno`; also check `upgrade` when protocol switching matters.
- `mulle-core/mulle-utf/mulle-utf-rover.h` initializes iterators with `(pointer, len)` and a sentinel, not a NUL terminator.
- `mulle-core/mulle-utf/TOC.md` separates low-level unchecked conversion from convenience `*_string` allocation helpers; verify the layer you actually changed.
- `mulle-core/mulle-time/TOC.md` warns against mixing calendar and absolute time in arithmetic.
- `mulle-core/mulle-time/mulle-time.h` omits `mulle-timeval.h` on Windows, so cross-platform checks must not assume that header is always available.
