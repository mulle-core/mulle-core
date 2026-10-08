## 0.10.0


feature: update amalgamated snapshots to 0.10.0 with new APIs

* add process spawning with ``mulle_spawn_options`` / ``mulle_spawn_open_with_options`,` plus ``mulle_spawn_interrupt/suspend/resume/detach`` and ``MULLE_SPAWN_DESCRIPTOR``
* add ASCII-only unicode predicates `(`mulle_unicode_is_asciidigit/asciiletter/asciispace/asciiidentifierstart/...`)` for deliberately ASCII lexers
* add container mutation pointers `(`*_get_mutation_pointer`)` for `NSFastEnumeration`-style change detection, plus ``mulle_pointerarray_remove_last``
* add C11 ``mulle_c_clz*`` / ``mulle_c_log2_floor`` helpers and ``MULLE_UTF32REGEX_EXTENDED`` character-class escapes and named classes
* concurrent hashtable now rejects out-of-range hashes; new ``mulle_concurrent_hashtable_hash_is_valid`` distinguishes rejection from absence
* add ``mulle_mmap_get_allocation_granularity`` and the ``mulle_allocator_fail_t`` noreturn typedef
* fix ``mulle_utf32_strcspn()`` for single-character search sets




* amalgamate tests into mulle-core so ppl can run ctest standalone


  absolute /opt/fil/bin/filcc path, drop job-level continue-on-error
  VM (vmactions/freebsd-vm); runs once per workflow run









* reorganize mulle-core for improved direct header inclusion with -isystem mulle-core/src


## 0.9.0









feature: migrate amalgamated library to updated mulle-core constituents

* new resizable wait-free ``mulle_concurrent_hashtable`` whose migration state lives in the hash word, so payloads are never destroyed during resize
* new ``mulle_arena`` bump allocator that subclasses ``mulle_allocator`` and can be passed to any allocator-consuming API
* slugify now offers UTF-8-preserving output `(`mulle_utf8_slugify_utf8`),` custom delimiters, and direct-to-buffer variants that keep non-transliterable letters
* new ``mulle_slugify`/`mulle_slugify_with_delimiter`` convenience functions write into a caller-provided buffer without allocation
* mulle-buffer gains flexible/inflexible backing constructors and ``mulle_buffer_do_*`` builder macros, plus a configurable default capacity
* allocators gain `reallocarray` (and strict) variants; string helpers and format strings are now fully `const`-qualified
* expanded C11 and mintomic atomic headers and updated thread portability layer


* new cmake compatible layout for the amalgamation, because why not


### 0.8.1



* **BREAKING** ``mulle_putchar`` signature changed: removed `FILE *fp` parameter, writes to stdout via `putchar()` instead of `fputc(c, fp)`
* -0.0 is no longer normalized to 0.0 in sprintf floating-point conversions; printed per C standard
* mulle-concurrent documentation rewritten with inline C function signatures for API reference












* Initial plan

* Add isolate-msvc-cl-crash.yml workflow for MSVC cl.exe crash isolation
