<!-- keywords: amalgamation, envelope, callbacks, concurrency, utf, http-parser, time -->

# mulle-core

Use this topic when you are coding against the amalgamated library instead of a single constituent. The local delta is that `mulle-core` is mostly an envelope: start at `<mulle-core/mulle-core.h>`, then drop to the constituent header that owns the allocator, callback, threading, or parsing rule you actually need.

## Understand first

```bash
mulle-sde api cat mulle-core
mulle-sde code search mulle_buffer_do_string
mulle-sde code search mulle_container_keycallback_copied_cstring
mulle-sde code search mulle_concurrent_hashmap_register
mulle-sde code search http_parser_execute
mulle-sde code search mulle_relativetime_now
```

## Local references

- `README.md`
- `asset/dox/TOC.md`
- `src/mulle-core.h`
- `src/mulle-buffer/mulle-buffer.h`
- `src/mulle-sprintf/mulle-sprintf.h`
- `src/mulle-container/mulle-array.h`
- `src/mulle-container/mulle-map.h`
- `src/mulle-container/mulle-container-callback-global.h`
- `src/mulle-concurrent/mulle-concurrent-hashmap.h`
- `src/mulle-thread/mulle-thread.h`
- `src/mulle-http/http_parser.h`
- `src/mulle-http/TOC.md`
- `src/mulle-utf/mulle-utf-rover.h`
- `src/mulle-url/_mulle-url-provide.h`
- `src/mulle-time/mulle-relativetime.h`
- `src/mulle-time/TOC.md`
- `test/README.md`

## Main local surfaces

| Scenario | Start here | Local rule |
| --- | --- | --- |
| Single include or version gate | `src/mulle-core.h` | Include the envelope header and use the version helpers there. |
| Temporary bytes or strings | `src/mulle-buffer/mulle-buffer.h`, `src/mulle-sprintf/mulle-sprintf.h` | Prefer scoped `*_do` helpers over manual create/destroy for transient work. |
| Arrays or maps with ownership | `src/mulle-container/mulle-array.h`, `src/mulle-container/mulle-map.h`, `src/mulle-container/mulle-container-callback-global.h` | Pick the callback scheme first; it defines hashing, equality, retain, and release. |
| Lock-free tables or one-time init | `src/mulle-concurrent/mulle-concurrent-hashmap.h`, `src/mulle-thread/mulle-thread.h` | Respect sentinel restrictions and cleanup rules; these APIs are explicit, not forgiving. |
| HTTP, URL, UTF, or time helpers | `src/mulle-http/http_parser.h`, `src/mulle-http/TOC.md`, `src/mulle-utf/mulle-utf-rover.h`, `src/mulle-url/_mulle-url-provide.h`, `src/mulle-time/mulle-relativetime.h` | Use length-based parsing and typed time helpers; do not assume NUL-terminated or single-shot inputs. |

## Primary local workflow

- Include `<mulle-core/mulle-core.h>` in consumer code, but read the constituent header before changing semantics.
- Treat `src/mulle-core.h` as the join point for the amalgamation. If you add another constituent, this is the file that must export it.
- Favor header comments and constituent `TOC.md` files over the top-level README for behavior. This repo ships the envelope and a sparse test harness, not many focused examples.

## Verify and report

- Name the constituent family you relied on, not just `mulle-core`.
- When a rule comes from ownership, callbacks, threading, or parsing, cite the exact header or `TOC.md` path above.
- If you changed the public envelope, say so explicitly; that is the repo-specific part other agents can easily miss.
