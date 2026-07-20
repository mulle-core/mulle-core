<!-- keywords: harness, consumer-build, smoke-test, cross-platform, mutation, sparse-corpus -->

# Coverage

Use this file when the task is coverage-oriented. The shared `coverage` howto has the generic commands; the local delta here is choosing the right constituent contract inside an amalgamated repo with a sparse local test corpus.

## Understand first

```bash
mulle-sde howto show coverage
mulle-sde howto show testing
mulle-sde api cat mulle-core
mulle-sde code search mulle_concurrent_hashmap_register
mulle-sde code search http_parser_execute
```

## Local references

- `src/mulle-core.h`
- `src/mulle-container/mulle-map.h`
- `src/mulle-concurrent/mulle-concurrent-hashmap.h`
- `src/mulle-http/http_parser.h`
- `src/mulle-utf/TOC.md`
- `src/mulle-time/TOC.md`
- `test/README.md`
- `test/cmake/toolchain--linux-windows--x86_64-w64-mingw32--mulle-clang.cmake`

## Coverage workflow

- Classify the change first: envelope export, ownership callback, concurrent sentinel rule, streaming parser rule, or time/UTF contract.
- Use `test/README.md` as the local harness contract: focused coverage additions here are small `.c` cases compiled against `mulle-core-test`.
- If the regression is about the public umbrella, make the test include `<mulle-core/mulle-core.h>` directly so you cover the amalgamation surface, not only a constituent header.
- If the regression is about consumer CMake or cross-platform wiring, keep `test/cmake/toolchain--linux-windows--x86_64-w64-mingw32--mulle-clang.cmake` in scope; that is the local fixture for Windows-style consumer verification.

## What good local coverage looks like

| Surface | Cover this | Avoid this |
| --- | --- | --- |
| Envelope/version | Compile or run through `<mulle-core/mulle-core.h>` and the version helpers in `src/mulle-core.h`. | Testing only the constituent header and missing a broken re-export. |
| Buffers/formatting | Check termination, allocator ownership, and scoped macro exit behavior. | Passing only "happy string" tests that never exercise overflow or cleanup. |
| Containers | Check callback-selected ownership, `notakey`, and the exact `insert`/`register`/`update`/`set` contract. | Treating all callbacks as `NULL`-sentinel string callbacks. |
| Concurrency/threading | Check invalid sentinel values, enumerator mutation alerts, and required cleanup. | Pure timing-based tests with no contract assertion. |
| HTTP/UTF/time | Check chunked or length-based inputs, buffer lifetime, and time-domain separation. | Single-shot NUL-terminated samples that miss the real streaming contract. |

## Local pitfalls

- This repo does not give you many ready-made focused tests. Small high-signal micro-tests are usually better than broad harness work.
- For HTTP verification, one callback is not one field. Fragmentation coverage matters more than a long fixture.
- For UTF coverage, distinguish low-level unchecked conversion from convenience allocating helpers; they have different failure surfaces.
- For time coverage, keep calendar, absolute, and relative types separate or the test can "pass" while checking the wrong invariant.
