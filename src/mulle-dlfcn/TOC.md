# mulle-dlfcn Library Documentation for AI
<!-- Keywords: dlfcn, dynamic, dlsym, dlopen, windows, crossplatform -->
## 1. Introduction & Purpose

- mulle-dlfcn is a small portability shim around the platform "dlfcn" API (dlopen/dlsym/...).
- It provides a unified macro for a default lookup handle (MULLE_RTLD_DEFAULT) and a helper to lookup symbols in the executable namespace (mulle_dlsym_exe).
- Solves cross-platform differences (Linux, macOS, Windows via dlfcn-win32) and documents the API consumers should use.
- Part of mulle-core; consumers normally link the entire mulle-core bundle rather than this project alone.

## 2. Key Concepts & Design Philosophy

- Minimal, header-first design: most useful helpers are inline in the public header to avoid extra linking overhead.
- Platform abstraction: the header chooses sensible defaults per-platform (MULLE_RTLD_DEFAULT), and exposes a single helper (mulle_dlsym_exe) to obtain symbols reliably from the executable namespace.
- Versioning via macros and tiny runtime getter (mulle_dlfcn_get_version).
- Intentionally tiny surface area: no memory ownership, no complex types — keeps the API easy for AIs and callers.

## 3. Core API & Data Structures

### 3.1. [mulle-dlfcn.h]

#### Version helpers & macro
- MULLE__DLFCN_VERSION
  - Purpose: Compile-time packed version (major<<20 | minor<<8 | patch).
- mulle_dlfcn_get_version(void)
  - Purpose: Returns the packed version as uint32_t (defined in .c).
- Inline helpers:
  - mulle_dlfcn_get_version_major(void)  — returns major
  - mulle_dlfcn_get_version_minor(void)  — returns minor
  - mulle_dlfcn_get_version_patch(void)  — returns patch

#### MULLE_RTLD_DEFAULT
- Purpose: Macro that expands to the platform-appropriate default handle for symbol lookups.
- Behavior:
  - If RTLD_DEFAULT is available, uses that.
  - On Linux defines 0 (executable namespace handle).
  - On macOS defines ((void *) -2).
  - On Windows defines ((void *) 0) (Windows uses dlfcn-win32 to emulate dlsym semantics).
- Usage: pass to dlsym or use in wrappers that require the default namespace.

#### mulle_dlsym_exe(const char *name)
- Purpose: Lookup a symbol by name searching at least the executable namespace; intended to avoid "DLL hell" where shared libraries link their own copy of dl functions.
- Signature: static inline void *mulle_dlsym_exe(const char *name)
- Behavior:
  - On Windows: uses GetModuleHandle(NULL) + GetProcAddress and returns a pointer to the symbol.
  - Otherwise: returns dlsym(MULLE_RTLD_DEFAULT, name).
- Notes: Returns NULL if the symbol was not found.

### 3.2. [mulle-dlfcn.c]

- Contains the single concrete function implementation:
  - uint32_t mulle_dlfcn_get_version(void) — returns MULLE__DLFCN_VERSION.
- Also defines a small file-scope symbol __MULLE_DLFCN_ranlib__ used for archive bookkeeping.

## 4. Performance Characteristics

- All operations are O(1) with respect to the symbol name beyond the underlying OS symbol lookup cost.
- No dynamic allocations; inline functions add no runtime overhead beyond the underlying platform calls.
- Thread-safety: the library itself is stateless and thread-safe; actual thread-safety for lookups depends on the platform's dlsym/GetProcAddress implementation (these are typically thread-safe).

## 5. AI Usage Recommendations & Patterns

- Prefer mulle_dlsym_exe when you need a reliable symbol lookup that searches the executable namespace (useful for plugin or runtime symbol resolution).
- Use MULLE_RTLD_DEFAULT for explicit dlsym calls when appropriate; it abstracts platform differences.
- On Windows, ensure <mulle-c11/mulle-c11-bool.h> is included before <windows.h> to avoid macro/typedef conflicts (the header enforces this with a compile-time error).
- Best practice: include only the public header (mulle-dlfcn.h). Do not rely on implementation internals.
- Common pitfall: do not assume MULLE_RTLD_DEFAULT has the same pointer value across platforms — use it symbolically.

## 6. Integration Examples

### Example 1: Get library version

```c
#include <stdio.h>
#include "mulle-dlfcn.h"

int
main()
{
   unsigned int   major;
   unsigned int   minor;
   unsigned int   patch;

   major = mulle_dlfcn_get_version_major();
   minor = mulle_dlfcn_get_version_minor();
   patch = mulle_dlfcn_get_version_patch();

   printf("mulle-dlfcn version %u.%u.%u\n", major, minor, patch);

   return( 0);
}
```

### Example 2: Lookup a function symbol from the executable and call it

```c
#include <stdio.h>
#include "mulle-dlfcn.h"

/* Example: find a function named "my_plugin_function" with signature void(void) */

int
main()
{
   void   (*fp)( void);
   void   *sym;

   sym = mulle_dlsym_exe("my_plugin_function");
   if( ! sym)
   {
      fprintf(stderr, "symbol not found\n");
      return( 1);
   }

   /* cast to the expected function pointer type and call */
   fp = (void (*)( void)) sym;
   fp();

   return( 0);
}
```

Notes on examples:
- Use the header-provided helpers and macros; do not attempt to replicate platform checks in consumer code.
- Examples are minimal and focused on correct API usage patterns.

## 7. Dependencies

- mulle-c11 (for basic cross-platform C macros and bool handling)
- dlfcn-win32 (on Windows, to provide dlsym/dlopen semantics)


---
Generated from public headers, README and tests.

- Tests: see test/00_link/nop.c which checks linking and that dlsym(MULLE_RTLD_DEFAULT, "main") resolves.
- Integration examples are present in the repository tests and in the header examples above.

