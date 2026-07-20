<!-- keywords: pitfalls, sentinels, ownership, callbacks, parser, mutex, monotonic -->

# Quirks

## The umbrella is not the behavior spec

- `src/mulle-core.h` is an export list plus version helpers. Do not infer constituent semantics from the top-level README alone.
- If you are changing a public coding rule, find the owning constituent header first.

## Buffer and sprintf pitfalls

- `src/mulle-sprintf/mulle-sprintf.h` says `mulle_buffer_sprintf` does **not** append `'\0'`.
- The same header says `mulle_snprintf` always writes a trailing zero but returns `-1` with `errno = ENOMEM` on overflow.
- `src/mulle-sprintf/mulle-sprintf.h` also says `mulle_asprintf` returns memory owned by the mulle allocator; free it with `mulle_free`, not plain `free`.
- `src/mulle-buffer/mulle-buffer.h` warns that `mulle_buffer_do_string` blocks must not `return`; use the macro’s extraction flow instead or you leak.

## Container ownership is encoded in the callback

- `src/mulle-container/mulle-array.h` says you must not insert `callback->notakey`; that sentinel is used for lookup failure and is not always `NULL`.
- `src/mulle-container/mulle-container-callback-global.h` shows that some presets use `NULL` as `notakey`, while integer and pointer-or-null presets use other sentinels. Do not assume one universal empty key value.
- `src/mulle-container/mulle-map.h` exposes four different write semantics: `insert`, `register`, `update`, and `set`. Do not collapse them into one mental model.
- `src/mulle-container/mulle-array.h` explicitly says integer-via-pointer callbacks work, but `mulle-structarray` is the preferred dense-integer container.

## Concurrent hashmap rules are strict

- `src/mulle-concurrent/mulle-concurrent-hashmap.h` forbids `hash = 0` and `value = 0` or `INTPTR_MIN`.
- The same header marks `mulle_concurrent_hashmap_patch` as experimental and less tested than the other operations.
- Its enumerator is thread-local and only limitedly tolerant of mutation: removals or growth can stop enumeration early.

## Thread helpers are scoped, but not infinitely nestable

- `src/mulle-thread/mulle-thread.h` says `mulle_thread_mutex_do` must not be nested inside another `mulle_thread_mutex_do`.
- The same header sets `MULLE_THREAD_MUTEX_NEEDS_DONE` to `1`; do not skip mutex cleanup on the assumption that the platform does it for you.

## HTTP and URL parsing are streaming APIs

- `src/mulle-http/http_parser.h` says callbacks may receive one logical field in many small chunks. Never assume `on_url` or header callbacks fire once per field.
- Callback pointers point into the current input buffer; if you need the data later, copy it while still in scope.
- Always compare the `http_parser_execute` return value with the input length and inspect `parser.http_errno` on short parse.
- `src/mulle-url/_mulle-url-provide.h` exports validators; use `http_parser_parse_url` from `src/mulle-http/http_parser.h` when you need URL offsets and fields.

## UTF and time helpers are length- and domain-sensitive

- `src/mulle-utf/mulle-utf-rover.h` initializes rovers with `(pointer, len)`. That API is sentinel-based, not NUL-terminated by default.
- `src/mulle-time/mulle-time.h` omits `mulle-timeval.h` on Windows.
- `src/mulle-time/TOC.md` warns against mixing calendar time with absolute or relative time in arithmetic; keep those domains separate.
