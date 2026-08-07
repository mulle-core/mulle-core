<!-- keywords: callbacks, sentinels, graphviz, stacktrace, streaming, ownership -->

# mulle-core

Use this topic after you already have a stacktrace or failing test and need the local interpretation step. `mulle-core` is mostly an envelope, so the first debugging move is to map the symptom from `mulle-core/mulle-core.h` to the owning constituent header.

## Understand first

```bash
mulle-sde howto show debug
mulle-sde api cat mulle-core
mulle-sde code search mulle_pointerarray_describe
mulle-sde code search mulle__rbtree_validate
mulle-sde code search mulle_concurrent_hashmapenumerator_next
mulle-sde code search http_parser_execute
```

## Local references

- `README.md`
- `asset/dox/TOC.md`
- `mulle-core/mulle-core.h`
- `mulle-core/mulle-sprintf/mulle-sprintf.h`
- `mulle-core/mulle-container/mulle-array.h`
- `mulle-core/mulle-container/mulle-map.h`
- `mulle-core/mulle-container/mulle-container-callback-global.h`
- `mulle-core/mulle-container-debug/mulle-pointerarray-debug.h`
- `mulle-core/mulle-rbtree-debug/mulle-rbtree-debug.h`
- `mulle-core/mulle-concurrent/mulle-concurrent-hashmap.h`
- `mulle-core/mulle-thread/mulle-thread.h`
- `mulle-core/mulle-http/TOC.md`
- `mulle-core/mulle-http/http_parser.h`
- `mulle-core/mulle-utf/mulle-utf-rover.h`
- `mulle-core/mulle-time/TOC.md`
- `test/README.md`

## Main local surfaces

| Symptom | Start here | Local read |
| --- | --- | --- |
| Missing export, wrong include, version mismatch | `mulle-core/mulle-core.h` | Confirm the constituent header is re-exported by the envelope before chasing implementation bugs. |
| Array/map corruption, ownership leaks, wrong sentinel | `mulle-core/mulle-container/mulle-array.h`, `mulle-core/mulle-container/mulle-map.h`, `mulle-core/mulle-container-debug/mulle-pointerarray-debug.h` | The callback preset defines retain/release, equality, hashing, and `notakey`; inspect that contract first. |
| Sorted structure corruption | `mulle-core/mulle-rbtree-debug/mulle-rbtree-debug.h` | Validate invariants first, then render ASCII or DOT if the shape matters. |
| Racy insert/remove or deadlock | `mulle-core/mulle-concurrent/mulle-concurrent-hashmap.h`, `mulle-core/mulle-thread/mulle-thread.h` | Sentinel restrictions and scoped mutex/once macros explain many local failures. |
| Short parses, bad slices, wrong timer math | `mulle-core/mulle-http/TOC.md`, `mulle-core/mulle-utf/mulle-utf-rover.h`, `mulle-core/mulle-time/TOC.md` | These APIs are length- and domain-driven; inspect lengths, fragments, and time-type mixups. |

## Primary local workflow

- Use the shared `debug` howto for getting the stacktrace. The repository-specific step starts when you identify the owning constituent.
- Prefer exported inspection helpers before reading implementations: `mulle_pointerarray_describe`, `mulle__rbtree_validate`, `mulle__rbtree_node_ascii_fprintf`, `mulle__rbtree_node_dot_fprintf`.
- For parser and time bugs, debug the contract, not just the crash frame. Short parses, chunked callbacks, ephemeral slices, sentinel values, and mixed time domains recur more often here than exotic algorithm bugs.
- `test/README.md` describes a generic harness, not a rich corpus of focused repros. Expect to make a small targeted test when the issue is local to one constituent.

## Verify and report

- Name the constituent family in the report, not only `mulle-core`.
- Say whether a helper returned borrowed state (`mulle__rbtree_validate`) or owned memory (`mulle_pointerarray_describe`).
- If the change touched `mulle-core/mulle-core.h`, call out that the public envelope changed.
