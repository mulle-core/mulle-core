<!-- keywords: envelope, buffers, formatting, containers, lock-free, parsing, ownership -->

# Patterns

## Start from the envelope, then narrow

- In app-facing code, prefer one include:

```c
#include <mulle-core/mulle-core.h>
```

- Use `src/mulle-core.h` for version checks and for the authoritative list of exported families.
- When behavior matters, open the constituent header next; the umbrella header does not redefine buffer, container, thread, or parser semantics.

## Use scoped buffers for transient output

- `src/mulle-buffer/mulle-buffer.h` favors scoped macros over manual lifetime code.
- `mulle_buffer_do` gives you a small stack-backed flexible buffer first and cleans it up automatically.
- `mulle_buffer_do_string` is the shortest path when you need an owned C string result.

```c
char  *s;

mulle_buffer_do_string( buffer, NULL, s)
{
   mulle_buffer_sprintf( buffer, "status=%d", rc);
   break;
}

fputs( s, stderr);
mulle_free( s);
```

- For short-lived formatted text with no explicit buffer variable, `src/mulle-sprintf/mulle-sprintf.h` offers `mulle_sprintf_do`.

## Pick the container callback before you pick the container

- `src/mulle-container/mulle-container-callback-global.h` exposes the common ownership presets: copied, nonowned, owned, pointer, and integer variants.
- Arrays embed one key callback plus the allocator. Maps embed a key/value callback pair plus the allocator.
- For C strings, start with `mulle_container_keycallback_copied_cstring` unless you have a good reason to keep external ownership.

```c
struct mulle_array  array = { 0 };

mulle_array_init_default( &array, &mulle_container_keycallback_copied_cstring);
mulle_array_add( &array, "alpha");
mulle_array_add( &array, "beta");
mulle_array_done( &array);
```

- For integers, `src/mulle-container/mulle-array.h` allows `mulle_container_keycallback_int` plus `mulle_int_as_pointer()` / `mulle_pointer_as_int()`, but the same header says `mulle-structarray` is the better fit for dense integer storage.
- For maps, use the operation that matches your duplicate policy from `src/mulle-container/mulle-map.h`: `insert` for fail-on-duplicate, `register` for "existing or new", `update` for replace-existing, `set` for unconditional convenience.

## Separate lock-free mutation from traversal

- `src/mulle-concurrent/mulle-concurrent-hashmap.h` is the local concurrent table surface.
- Initialize once, then use `mulle_concurrent_hashmap_register` or `mulle_concurrent_hashmap_insert` for publication.
- Keep enumeration isolated from mutation-heavy code paths; the header only promises a limited, thread-local enumerator.
- For one-time setup around those structures, prefer the block form from `src/mulle-thread/mulle-thread.h`:

```c
mulle_thread_once_do( once_token)
{
   /* initialize shared state */
}
```

## Treat HTTP, URL, UTF, and time as length-driven helpers

- `src/mulle-http/http_parser.h` and `src/mulle-http/TOC.md` show the normal parser shape: fill `http_parser_settings`, call `http_parser_init`, feed chunks with `http_parser_execute`, then inspect `parser.http_errno` if parsing stops early.

```c
struct http_parser           parser;
struct http_parser_settings  settings = { 0 };
size_t                       parsed;

http_parser_init( &parser, HTTP_REQUEST);
parsed = http_parser_execute( &parser, &settings, request, strlen( request));
if( parsed < strlen( request))
   fprintf( stderr, "%s\n", http_errno_name( parser.http_errno));
```

- Use `http_parser_parse_url` for actual URL component extraction. `src/mulle-url/_mulle-url-provide.h` is mainly the character-validation layer that supports parsing.
- Use `src/mulle-utf/mulle-utf-rover.h` when you need one iterator shape across UTF-8, UTF-16, and UTF-32:

```c
struct mulle_utf_rover  rover;

_mulle_utf8_rover_init( &rover, s, len);
while( _mulle_utf_rover_has_character( &rover))
   consume( _mulle_utf_rover_next_character( &rover));
```

- Use typed time helpers from `src/mulle-time/mulle-relativetime.h` and `src/mulle-time/TOC.md` to keep duration, monotonic time, and calendar time separate.
