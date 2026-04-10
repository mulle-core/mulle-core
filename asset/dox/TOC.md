# mulle-core Library Documentation for AI
<!-- Keywords: amalgamation, envelope, version, headers, dependencies -->

## 1. Introduction & Purpose

- mulle-core is an amalgamated convenience library that bundles many small C libraries (allocator, buffer, container, concurrent, rbtree, regex, utf, url, time, etc.) behind a single envelope header (<mulle-core/mulle-core.h>).
- Solves the problem of repeatedly pulling and linking many small libraries: provide a single include and link target (-lmulle-core) for quicker builds and simpler dependency management.
- Key features: single envelope header, version macros/functions, and aggregated APIs from constituents.

## 2. Key Concepts & Design Philosophy

- Envelope/Amalgamation: mulle-core exposes one top-level header that includes many component headers so user code can #include <mulle-core/mulle-core.h> and use constituents without individual linkage management.
- Minimal glue: does not reimplement constituent APIs, only re-exports and centralizes versions and include paths.
- Best-effort portability: relies on mulle-sde/mulle-c11 for cross-platform compile-time glue.

## 3. Core API & Data Structures

### 3.1. [mulle-core.h]

#### Overview
- Purpose: The single public envelope header for the amalgamation. It provides version helpers and includes the public headers of many constituent libraries.
- Top-level symbols in this header (most other APIs come from included headers):

  - MULLE__CORE_VERSION (macro)
    - Encodes major/minor/patch in a 32-bit value.
  - mulle_core_get_version_major(void)
    - Inline helper returning major version.
  - mulle_core_get_version_minor(void)
    - Inline helper returning minor version.
  - mulle_core_get_version_patch(void)
    - Inline helper returning patch version.
  - uint32_t mulle_core_get_version(void)
    - Global function returning the combined version value.

#### Purpose & Usage
- Include this header to gain access to all constituent public headers without per-library include statements.
- Use the version helpers for runtime or compile-time checks.

### 3.2. Constituent headers (short list)
- The envelope includes (non-exhaustive):
  - mulle-allocator — flexible allocation schemes
  - mulle-buffer — growable char buffer and stream
  - mulle-container — arrays, hashtables, queues
  - mulle-concurrent / mintomic / mulle-aba — lock- and wait-free concurrent primitives
  - mulle-rbtree — balanced binary tree APIs
  - mulle-regex — Unicode-aware regex parser
  - mulle-url, mulle-utf, mulle-unicode — URL and Unicode helpers
  - mulle-sprintf / mulle-fprintf — formatting helpers
  - mulle-thread — cross-platform threading and synchronization

- For full API surface, consult each constituent's public headers under src/<component>/ (this envelope does not redefine constituent function signatures).

## 4. Performance Characteristics

- Performance depends on the individual constituent libraries. General guidance:
  - Containers (arrays, queues): typically O(1) append, O(n) iteration.
  - Hash tables (mulle-container): average-case O(1) lookup/insert, worst-case O(n).
  - RB-tree (mulle-rbtree): O(log n) insert/search/remove.
  - Concurrent primitives (mulle-concurrent, mintomic): designed for lock-free or wait-free operation where advertised; consult those libraries for memory ordering and ABA-safety.
- Trade-offs: amalgamation improves build/link simplicity at the cost of a larger single binary/library. Thread-safety is component-dependent; do not assume global thread-safety.

## 5. AI Usage Recommendations & Patterns

- Best Practices:
  - Use #include <mulle-core/mulle-core.h> for convenience when the amalgamated set of libraries is acceptable.
  - Prefer constituent lifecycle functions (alloc/free, init/done) as documented in each header.
  - Use the version helpers to guard against incompatible API changes.
- Common Pitfalls:
  - Do not assume every included API is thread-safe; check the specific component.
  - Avoid poking at internal or underscore-prefixed fields from constituents; use public accessors.
  - Remember that some constituents may require additional link flags or initialization (see their README/testing docs).
- Idiomatic "mulle-sde" usage:
  - Add the library with `mulle-sde add github:mulle-core/mulle-core` or use it as a CMake subdirectory and link with `target_link_libraries(... mulle-core)`.

## 6. Integration Examples

### Example 1: Including the envelope and checking version

```c
#include <mulle-core/mulle-core.h>
#include <stdio.h>

int
main()
{
   unsigned int  major;

   major = mulle_core_get_version_major();
   printf("mulle-core major version: %u\n", major);
   return( 0);
}
```

### Example 2: Creating and using a mulle-buffer (constituent API)

```c
#include <mulle-core/mulle-core.h>
#include <stdio.h>

int
main()
{
   struct mulle_buffer  *buf;

   buf = mulle_buffer_create( 0);
   mulle_buffer_add_string( buf, "hello");
   printf("buffer: %s\n", mulle_buffer_cstring( buf));
   mulle_buffer_destroy( buf);
   return( 0);
}
```

Notes: examples use the constituent APIs re-exported by the envelope header. Consult the specific header for lifecycle semantics and return value ownership.

## 7. Dependencies

- Direct constituents (selected from .mulle/etc/sourcetree/config):
  - mulle-allocator
  - mulle-buffer
  - mulle-c11
  - mulle-container
  - mulle-data
  - mulle-http
  - mulle-rbtree
  - mulle-regex
  - mulle-slug
  - mulle-storage
  - mulle-unicode
  - mulle-url
  - mulle-utf
  - mulle-vararg
  - mulle-thread
  - mulle-concurrent (and mintomic / mulle-aba)
  - mulle-mmap
  - mulle-fprintf
  - mulle-sprintf
  - mulle-time

- The envelope delegates to each constituent for detailed behavior and implementation.

---

References: README.md (project overview), src/mulle-core.h (envelope header), .mulle/etc/sourcetree/config (dependency list).