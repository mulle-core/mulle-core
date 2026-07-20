<!-- keywords: amalgamation, envelope, callbacks, concurrency, streaming, cross-build -->

# mulle-core

Use this topic when you are verifying changes against the amalgamated library instead of a single constituent. The local delta is that `mulle-core` is mostly an envelope: first identify the owning family from `src/mulle-core.h`, then verify that family's real contract in its own header or `TOC.md`.

## Understand first

```bash
mulle-sde howto show testing
mulle-sde howto show coverage
mulle-sde api cat mulle-core
mulle-sde code search mulle_container_keycallback_copied_cstring
mulle-sde code search mulle_concurrent_hashmap_register
mulle-sde code search http_parser_execute
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
- `src/mulle-utf/TOC.md`
- `src/mulle-time/mulle-time.h`
- `src/mulle-time/TOC.md`
- `test/README.md`
- `test/cmake/toolchain--linux-windows--x86_64-w64-mingw32--mulle-clang.cmake`

## Main local surfaces

| What changed | Start here | Verify for |
| --- | --- | --- |
| Envelope export or version gate | `src/mulle-core.h` | The family is still re-exported and the version helper path still compiles through `<mulle-core/mulle-core.h>`. |
| Buffer or formatting code | `src/mulle-buffer/mulle-buffer.h`, `src/mulle-sprintf/mulle-sprintf.h` | Ownership, `'\0'` handling, and scoped macro cleanup. |
| Array or map behavior | `src/mulle-container/mulle-array.h`, `src/mulle-container/mulle-map.h`, `src/mulle-container/mulle-container-callback-global.h` | Callback-selected ownership, `notakey`, and duplicate-policy semantics. |
| Lock-free or thread helper code | `src/mulle-concurrent/mulle-concurrent-hashmap.h`, `src/mulle-thread/mulle-thread.h` | Sentinel restrictions, enumerator limits, and required cleanup. |
| HTTP, URL, UTF, or time helpers | `src/mulle-http/http_parser.h`, `src/mulle-http/TOC.md`, `src/mulle-utf/mulle-utf-rover.h`, `src/mulle-utf/TOC.md`, `src/mulle-time/TOC.md` | Length-driven parsing, fragmented callbacks, and time-domain separation. |

## Primary local workflow

- Start at `src/mulle-core.h` to confirm which constituent family is actually public through the envelope.
- Use the shared `testing` and `coverage` howtos for the mechanics. The repository-specific step is picking the right constituent contract to verify.
- `test/README.md` describes a generic `.c`-file harness, but this repo ships very little focused local test content. Expect umbrella verification to be a small targeted smoke test, not a rich existing corpus.
- If the change affects consumer builds or install-layout assumptions, also treat `test/cmake/toolchain--linux-windows--x86_64-w64-mingw32--mulle-clang.cmake` as a local cross-build reference.

## Verify and report

- Name the constituent family you verified, not only `mulle-core`.
- Cite the exact header or `TOC.md` path that supplied the rule you checked.
- Call out envelope changes in `src/mulle-core.h` explicitly; those are the repo-specific regressions other agents miss most easily.
