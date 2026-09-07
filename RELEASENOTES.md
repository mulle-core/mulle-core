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
