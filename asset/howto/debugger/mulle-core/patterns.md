<!-- keywords: ownership, sentinels, graphviz, incremental, mutation, ranges -->

# Patterns

## Start at the envelope, then jump to the owner

- If a symbol is missing or a failure started after an include/export change, inspect `src/mulle-core.h` first.
- The umbrella header gives you the family list and version helpers. The real debugging rules live in the owning constituent header or `TOC.md`.

## Check ownership and terminators on formatted output

- `src/mulle-sprintf/mulle-sprintf.h` says `mulle_snprintf` always appends `'\0'`, but still returns `-1` with `errno = ENOMEM` on overflow.
- `src/mulle-buffer/mulle-buffer.h` and `src/mulle-sprintf/TOC.md` say `mulle_buffer_sprintf` does **not** append `'\0'`.
- `mulle_asprintf` and `mulle_buffer_do_string` hand back mulle-allocated strings; release them with `mulle_free`.

```c
char   *s;

if( mulle_asprintf( &s, "state=%d", state) == 0)
{
   fputs( s, stderr);
   mulle_free( s);
}
```

## Dump container state before single-stepping internals

- `src/mulle-container-debug/mulle-pointerarray-debug.h` is the concrete inspection helper shipped here.
- `mulle_pointerarray_describe` returns an owned string; print it and free it with `mulle_free`.
- If addresses are not enough, switch to `mulle_pointerarray_describe_buffer_callback` and print each item with the same callback assumptions as the container.

```c
char   *dump;

dump = mulle_pointerarray_describe( &array);
fputs( dump, stderr);
mulle_free( dump);
```

## Validate tree invariants before blaming callers

- `src/mulle-rbtree-debug/mulle-rbtree-debug.h` gives you the low-level probes: validate, ASCII, and DOT output.
- `mulle__rbtree_validate` returns `NULL` for a valid tree or a static error string describing the broken invariant.
- Use ASCII first in terminal sessions; use DOT when parent/child shape is the question.

```c
char   *reason;

reason = mulle__rbtree_validate( &raw_tree);
if( reason)
{
   fprintf( stderr, "rbtree: %s\n", reason);
   mulle__rbtree_node_ascii_fprintf( stderr, &raw_tree, NULL);
}
```

## Keep concurrent repros honest

- Build the repro around the actual contract in `src/mulle-concurrent/mulle-concurrent-hashmap.h`: `hash != 0`, `value != NULL`, `value != (void *) INTPTR_MIN`.
- When enumerating, keep the return code so mutation alerts stay visible. `ECANCELLED` means the rover stopped on concurrent mutation or growth, not "map empty".
- If the stall sits inside scoped locking, inspect for nested `mulle_thread_mutex_do`. For intended re-entry, test with `mulle_thread_recursive_mutex_do` or `mulle_thread_once_do_recursive` instead of nesting the plain macro.

```c
int      rval;
intptr_t hash;
void     *value;

mulle_concurrent_hashmap_for_rval( map, hash, value, rval)
{
   inspect_pair( hash, value);
}
if( rval == ECANCELLED)
   fputs( "enumeration stopped on concurrent mutation\n", stderr);
```

## Parse incrementally and preserve slices explicitly

- `http_parser_execute` is the normal entry point. Compare its return value with the input length on every feed.
- Treat `on_url`, `on_header_field`, and `on_header_value` as fragment callbacks; accumulate them into your own buffer.
- Copy `at` slices before the next feed if you need them later, then run `http_parser_parse_url` on the complete URL buffer.

```c
size_t   parsed;

parsed = http_parser_execute( &parser, &settings, chunk, len);
if( parsed < len)
   fprintf( stderr, "%s\n", http_errno_name( parser.http_errno));
```

## Debug text and time with lengths and domains

- Initialize UTF rovers with `(pointer, len)` and step with `_mulle_utf_rover_has_character`; if text looks truncated, inspect the passed length before hunting terminator bugs.
- For time issues, compare like with like: absolute/monotonic for elapsed time, calendar for wall-clock timestamps, relative for durations.
- If the platform is Windows, remember `src/mulle-time/mulle-time.h` does not include `mulle-timeval.h`.
