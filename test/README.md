# Vendored constituent tests

This directory carries the behavioral tests of the amalgamated constituent
projects so that a plain CMake checkout can exercise the combined library with
`ctest`, without requiring mulle-sde.

## Layout

```
test/<constituent>/<...>/<name>.c
```

The tree is **not** flattened. `default.*` support files and `.skip.*` markers
apply to everything in the directory they live in, and test basenames are only
unique within a directory (for example `mulle-mmap/20_file_mapping/test.c`
and `mulle-mmap/60_memory_management/test.c`).

The files are snapshots. The canonical source for every test is the owning
constituent project; fix a test there and re-vendor it here.

## Re-vendoring

`test/amalgamate-tests.sh` copies the test trees of all amalgamated
constituents and rewrites `test/VENDORED` with the revisions it took. The
constituent repositories live in several roots of the mulle-core meta-repo:

```sh
test/amalgamate-tests.sh
test/amalgamate-tests.sh /path/to/mulle-core:/path/to/mulle-c
MULLE_CORE_CONSTITUENT_ROOTS=/a:/b test/amalgamate-tests.sh
test/amalgamate-tests.sh --dry-run
```

By default it searches `<this repo>/../..`, `/home/src/srcO/mulle-c` and
`/home/src/srcO/mulle-concurrent`. It aborts before deleting anything if a
constituent repository cannot be found (pass `--allow-missing` to skip those).

The script skips a handful of tests that cannot be vendored as-is; see
`SKIP_TESTS` in the script. It also skips every test that includes
`mulle-testallocator` (see below).

## Running

The suite is off by default, so that building the amalgamation as a mulle-sde
dependency does not build it. Opt in explicitly, and never as an
`add_subdirectory()` subproject (the tests are then not registered):

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Each `.c` file becomes one CTest test, named by its path relative to `test/`.
Tests are compiled and linked against the `mulle-core` target (plus `m`,
`pthread`, `dl` and `rt` where needed).

Every test runs in a private copy of its directory under the build tree, so
tests that create, chmod or delete files next to themselves do not mutate the
vendored snapshot. Tests in `*multithreaded*`/`*stress*` directories are
registered `RUN_SERIAL` because their output depends on thread scheduling.

## Support files

This mirrors the `mulle-test` conventions that `mulle-sde test` uses. See
`cmake/RunConstituentTest.cmake` for the exact behaviour.

| File | Meaning |
|------|---------|
| `<name>.stdout` | expected standard output |
| `<name>.stderr` | expected standard error, whitespace-insensitive |
| `<name>.errors` | each line is a pattern that must appear on stderr; its presence means a non-zero exit is expected |
| `<name>.stdin` | fed to standard input |
| `<name>.args` | arguments, one per line (`#` comments allowed) |
| `<name>.environment` | `export KEY=VALUE` / `KEY=VALUE` lines |
| `<name>.cat` | script that massages the actual output before comparing |
| `<name>.diff` | script that compares expected and actual itself (used by the multithreaded tests to sort their output) |
| `<name>.skip` / `.skip.<platform>` / `.skip.<platform>.<arch>` | do not register the test |
| `no-mulle-test[.<platform>]` | do not register any test in the directory |

`<name>` may be replaced by `default` to share a support file with the whole
directory. Platform (`linux`, `darwin`, `windows`) and architecture variants
are preferred when present, e.g. `<name>.stdout.windows`.

Whitespace-only differences in stdout are reported as a warning but do not fail
the test; this matches `mulle-test`'s default
(`MULLE_TEST_WHITESPACE_DIFFERENCES=warn`).

## `include.h`

Tests that do `#include "include.h"` get the shared `test/include.h`, which is
just `<mulle-core/mulle-core.h>`. `mulle-sde test` generates an equivalent file
per test project.

## Not vendored

- **mulle-testallocator dependency.** The constituent test sources used to
  `#include <mulle-testallocator/mulle-testallocator.h>` and install it as the
  leak-checking allocator. Since mulle-testallocator is deliberately not part
  of the amalgamation, that boilerplate (`*_initialize`, `*_reset`,
  `*_set_tracelevel`, `mulle_default_allocator = mulle_testallocator`) was
  removed from the owning constituent test sources and the vendored snapshots.
  The few tests that used `&mulle_testallocator` as an allocator now use
  `&mulle_allocator_default`. The consequence is that these tests no longer do
  leak checking; the constituent projects' own `mulle-sde test` runs are
  affected too. `amalgamate-tests.sh` still skips any test that includes the
  header, as a guard.
- **Compile-failure tests.** A test with a `<name>.ccdiag` companion is meant
  to fail compilation. The driver does not implement compiler-diagnostic
  matching yet, so those are skipped at configure time.
- **Individual incompatible tests.** A few tests are layout-, revision- or
  toolchain-dependent; they are listed with reasons in `SKIP_TESTS` in
  `amalgamate-tests.sh`.