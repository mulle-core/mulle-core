### 0.8.1



* **BREAKING** ``mulle_putchar`` signature changed: removed `FILE *fp` parameter, writes to stdout via `putchar()` instead of `fputc(c, fp)`
* -0.0 is no longer normalized to 0.0 in sprintf floating-point conversions; printed per C standard
* mulle-concurrent documentation rewritten with inline C function signatures for API reference












* Initial plan

* Add isolate-msvc-cl-crash.yml workflow for MSVC cl.exe crash isolation
