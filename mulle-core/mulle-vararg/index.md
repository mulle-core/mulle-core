# mulle-vararg Library Documentation for AI
<!-- Keywords: varargs, variadic-functions -->

## 1. Introduction & Purpose

mulle-vararg is an alternative to standard `<stdarg.h>` for handling variable arguments in C. Instead of stack-based variadic argument access (which varies by architecture), mulle-vararg uses struct-based layout where all arguments are packed into a contiguous buffer as if they were struct fields. This provides a consistent, portable API for variable argument handling across architectures while maintaining C argument promotion rules. This is a specialized utility in the mulle-c ecosystem for implementing variadic functions in compiler-like contexts.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Struct-Based Layout:** Arguments packed into memory as struct fields, not as stack frames.

- **Architecture-Independent:** Eliminates architecture-specific ABI concerns (x86 cdecl, ARM EABI, etc.).

- **Alignment Aware:** Respects C alignment rules for field placement (e.g., 8-byte alignment for doubles).

- **Promotion Aware:** Honors C argument promotion rules (char→int, float→double).

- **Portable:** Works consistently across 32-bit and 64-bit platforms without recompilation.

- **Manual or Automatic:** Can be used with compiler support (automatic packing via the mulle-clang metaABI) or manually via the builder API.

## 3. Core API & Data Structures

### 3.1 `mulle-vararg.h` - Core Vararg Access

#### Types

**`mulle_vararg_list`**

- **Purpose:** Opaque cursor for accessing variable arguments.
- **Internal:** Holds a pointer to the current position in the argument buffer.
- **Not compatible** with `stdarg.h`'s `va_list`.

#### Initialization Macros

**`_mulle_vararg_start(args, lvalue)`** — the portable entry point.

- **Purpose:** Initialize an argument list at the first byte *after* the given lvalue (a struct field or local variable).
- **Parameters:**
  - `args`: `mulle_vararg_list` to initialize.
  - `lvalue`: Any lvalue; the list starts after `sizeof(lvalue)` bytes (with an `int`-sized minimum, per C promotion rules).
- **Usage:** Use it when you lay out arguments manually (struct fields, builder buffers).
- **Example:** `_mulle_vararg_start(list, value.a)` starts at the field after `value.a`.

**`_mulle_vararg_start_fp(args, lvalue)`**

- **Purpose:** Like `_mulle_vararg_start`, but for lists whose first argument is floating point (`double`-sized minimum slot).

**`mulle_vararg_start(args, ap)`** and **`mulle_vararg_start_fp(args, ap)`**

- **Purpose:** Initialize an argument list inside a variadic function, where `ap` is the last named parameter.
- **⚠️ metaABI only:** These expand to `_mulle_vararg_start(args, _param->ap)` and `_mulle_vararg_start_fp(args, _param->ap)` respectively. They compile **only** under the mulle-clang metaABI (mulle-objc runtime), which injects a `_param` struct pointer, and only when the last named parameter is *literally named* `ap` (the name is used as a macro token).
- **Portable alternative:** Always prefer `_mulle_vararg_start(args, &last_param)`-style usage (on the lvalue) for code compiled with plain gcc/clang.

#### Integer Argument Access

**`mulle_vararg_next_integer(args, type)`**

- **Purpose:** Read and advance to next integer-like argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` iterator.
  - `type`: C integer type to extract (int, char, long, etc.).
- **Returns:** Value converted to specified type.
- **Behavior:** Handles integer promotion (small types read from an `int`-sized slot).

#### Floating-Point Argument Access

**`mulle_vararg_next_fp(args, type)`**

- **Purpose:** Read and advance to next floating-point argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` iterator.
  - `type`: float, double or long double.
- **Returns:** Value converted to specified type.
- **Behavior:** `float` is read from a `double`-sized slot (promotion).

**`mulle_vararg_next_double(args)`, `mulle_vararg_next_float(args)`, `mulle_vararg_next_longdouble(args)`**

- Convenience wrappers around `mulle_vararg_next_fp`.

#### Pointer Argument Access

**`mulle_vararg_next_pointer(args, type)`**

- **Purpose:** Read and advance to next pointer argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` iterator.
  - `type`: Pointer type (e.g., `void *`, `char *`).
- **Returns:** Pointer value.

#### Struct and Union Access

**`mulle_vararg_next_struct(args, type)`** / **`mulle_vararg_next_union(args, type)`**

- **Purpose:** Read the next struct/union **by value** (copied out of the buffer).

**`_mulle_vararg_next_struct(args, type)`** / **`_mulle_vararg_next_union(args, type)`**

- **Purpose:** Get a **pointer into** the buffer for the next struct/union (no copy; more efficient, but the data is only valid while the buffer lives).

#### Convenience Read Macros

`mulle_vararg_next_char`, `next_short`, `next_int`, `next_int32`, `next_int64`, `next_long`, `next_longlong`, `next_unsignedchar`, `next_unsignedshort`, `next_unsignedint`, `next_uint32`, `next_uint64`, `next_unsignedlong`, `next_unsignedlonglong` (all delegate to `mulle_vararg_next_integer`).

#### List Management

**`mulle_vararg_copy(dst, src)`** — copy a list cursor (e.g., to iterate twice).

**`mulle_vararg_end(args)`** — end marker (currently a no-op).

**`mulle_vararg_count_pointers(args, first)`** — count a NULL-terminated list of pointer arguments (including `first`).

### 3.2 `mulle-vararg-builder.h` - Manual Argument Construction

The builder constructs a vararg buffer manually (no compiler support needed) and can hand it to any consumer of `mulle_vararg_list` (e.g. `mulle_mvsprintf`). The API is **macro-based** — there is no builder struct.

#### Buffer Type and Sizing

**`mulle_vararg_builderbuffer_t`**

- **Purpose:** Buffer element type (`long double`). Its alignment satisfies the strictest argument alignment (`alignof(long double)`).
- **Usage:** `mulle_vararg_builderbuffer_t buf[ mulle_vararg_builderbuffer_n( size)];`

**`mulle_vararg_builderbuffer_n(n)`**

- **Purpose:** Number of `mulle_vararg_builderbuffer_t` elements needed to hold `n` bytes.

**Sizing macros:** `mulle_vararg_sizeof_integer(type)`, `mulle_vararg_sizeof_fp(type)`, `mulle_vararg_sizeof_pointer(type)`, `mulle_vararg_sizeof_functionpointer(type)`, `mulle_vararg_sizeof_struct(type)` and matching `mulle_vararg_alignof_*` macros compute the promoted slot size/alignment of an argument. Convenience forms exist: `mulle_vararg_sizeof_int()`, `mulle_vararg_sizeof_long()`, `mulle_vararg_sizeof_float()` (returns `sizeof(double)`), `mulle_vararg_sizeof_double()`, etc.

#### Push Macros

**`mulle_vararg_push_integer(ap, type, value)`**

- **Purpose:** Push an integer argument.
- **Behavior:** Respects promotion (small types stored in an `int`-sized slot).

**`mulle_vararg_push_fp(ap, type, value)`**

- **Purpose:** Push a floating-point argument (`float` stored in a `double`-sized slot).

**`mulle_vararg_push_struct(ap, value)`** / **`mulle_vararg_push_union(ap, value)`**

- **Purpose:** Push a struct/union by value.

**`mulle_vararg_push_pointer(ap, value)`** / **`mulle_vararg_push_functionpointer(ap, value)`**

- **Purpose:** Push a pointer / function pointer.

**Convenience push macros:** `mulle_vararg_push_char`, `push_short`, `push_int`, `push_int32`, `push_int64`, `push_long`, `push_longlong`, `push_unsigned*`, `push_uint32`, `push_uint64`, `push_float`, `push_double`, `push_longdouble`.

#### List Creation

**`mulle_vararg_list_make(buf)`**

- **Purpose:** Turn a buffer into a `mulle_vararg_list` pointing at its start. Pass the result to a consumer (e.g. `mulle_mvsprintf`).

**`mulle_vararg_builder_do(name, size)`**

- **Purpose:** Allocate the buffer on the stack and declare the list `name` in one step. Expands to `mulle_alloca_do` from mulle-allocator — requires linking `mulle-allocator`.

**Important:** the caller is responsible for sizing the buffer correctly with the `mulle_vararg_sizeof_*`/`mulle_vararg_alignof_*` macros. There is no bounds checking; an undersized buffer is undefined behavior.

### 3.3 `mulle-align.h` - Alignment Utilities

**`mulle_pointer_align(p, align)`**

- **Purpose:** Align pointer to specified alignment boundary.
- **Parameters:**
  - `p`: Pointer to align.
  - `align`: Alignment (typically power of 2).
- **Returns:** Aligned pointer (may be ≥ original).

**`mulle_address_align(p, align)`** — same, on a `uintptr_t`.

## 4. Argument Layout Examples

### Example 1: Simple Integer Arguments

```
printf("x=%d", 42)

Argument buffer:
[0x00] 00000000   (unused - first named argument slot)
[0x04] 0000002A   (int 42)
```

### Example 2: Mixed Types with Promotion

```
printf("%d %f", (char)'x', (float)0.2)

Argument buffer (32-bit):
[0x00] 00000078                 (int slot, char 'x' promoted)
[0x04] 00000000
[0x08] 3fc99999 9999999a       (double slot, float 0.2 promoted)
```

### Example 3: Pointer and Large Integer

```
printf("%p %lld", ptr, 1848LL)

Argument buffer (64-bit):
[0x00] [pointer_value]         (void * 8 bytes)
[0x08] [long long 1848]        (long long 8 bytes)
```

## 5. Performance Characteristics

- **Access Time:** O(1) per argument; sequential traversal.
- **Memory:** No allocation overhead; uses provided buffer.
- **Alignment:** Minimal padding for proper alignment.
- **Predictability:** No architecture-specific variations.

## 6. AI Usage Recommendations & Patterns

### Best Practices:

1. **Use the builder for construction:** To *construct* a list portably, always use the builder (`mulle_vararg_list_make` + `mulle_vararg_push_*`). Do not invent a builder struct — there is none.

2. **Initialize correctly:** Use `_mulle_vararg_start(list, lvalue)` with an lvalue (field/local). Use `mulle_vararg_start` only in mulle-clang metaABI code where `_param` is injected.

3. **Type Safety:** Know expected argument types; mismatched reads yield garbage.

4. **Buffer Size:** Builder users must size the buffer with `mulle_vararg_builderbuffer_n` + the `mulle_vararg_sizeof_*` macros; an undersized buffer is UB.

5. **Alignment Respect:** Trust the alignment macros; manual pointer arithmetic risks misalignment.

### Common Pitfalls:

1. **Accessing Arguments Out-of-Order:** Sequential access required; no random access.

2. **Type Mismatch:** Reading with the wrong macro (e.g., `next_integer` instead of `next_fp`) yields corrupted values.

3. **Buffer Overflow (Builder):** Insufficient buffer causes corruption; always size with the sizing macros.

4. **Assuming Stack Layout:** Not compatible with standard stdarg; don't mix APIs.

5. **Forgetting Promotion Rules:** Small integers are promoted to int, floats to double; account for this when reading and when sizing buffers.

6. **Using `mulle_vararg_start` in plain C:** It requires the metaABI `_param`; use `_mulle_vararg_start` instead.

## 7. Integration Examples

### Example 1: Reading a List Built by the Builder (portable)

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>

int main(void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n(
                                          mulle_vararg_sizeof_integer( int) +
                                          mulle_vararg_sizeof_fp( double) +
                                          mulle_vararg_sizeof_pointer( char *))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   int     i;
   double  d;
   char    *s;

   mulle_vararg_push_int( q, 42);
   mulle_vararg_push_double( q, 3.14);
   mulle_vararg_push_pointer( q, "hello");

   // read in separate statements: argument evaluation order is unspecified
   i = mulle_vararg_next_integer( list, int);
   d = mulle_vararg_next_fp( list, double);
   s = mulle_vararg_next_pointer( list, char *);
   printf( "%d %.2f %s\n", i, d, s);
   return 0;
}
```

### Example 2: Passing a Built List to mulle_mvsprintf

```c
#include <mulle-vararg/mulle-vararg.h>
#include <mulle-sprintf/mulle-sprintf.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n(
                                          mulle_vararg_sizeof_integer( int) +
                                          mulle_vararg_sizeof_integer( long))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;
   char                          out[ 64];

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);
   mulle_vararg_push_int( q, 18);
   mulle_vararg_push_long( q, 48L);

   mulle_mvsprintf( out, "%d %ld", list);
   printf( "%s\n", out);   // prints: 18 48
   return 0;
}
```

### Example 3: Reading Manually Laid Out Struct Fields (portable)

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>

struct parameters
{
   int      count;
   double   average;
   char     *label;
};

int main(void)
{
   struct parameters  params = { 10, 4.5, "measurements" };
   mulle_vararg_list  list;

   double   average;
   char     *label;

   // start AFTER the first field: the "varargs" are average and label
   _mulle_vararg_start( list, params.count);

   average = mulle_vararg_next_fp( list, double);
   label   = mulle_vararg_next_pointer( list, char *);
   printf( "Count: %d, Average: %.2f, Label: %s\n", params.count, average, label);
   return 0;
}
```

### Example 4: MetaABI Variadic Function (mulle-clang / mulle-objc only)

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>

// Compiles only under the mulle-clang metaABI: the compiler injects a
// `_param` struct pointer, and the last named parameter must be named `ap`.
void   myprintf( char *ap, ...)
{
   mulle_vararg_list   list;

   mulle_vararg_start( list, ap);

   printf( "%d\n", mulle_vararg_next_integer( list, int));
   printf( "%.1f\n", mulle_vararg_next_fp( list, double));
}

int   main( void)
{
   myprintf( "x", 18, 48.0);
   return 0;
}
```

## 8. Dependencies

Direct dependencies (for standalone use):
- `mulle-c11`: C11 compatibility macros and alignment utilities
- `mulle-allocator`: provides `mulle_alloca_do` used by `mulle_vararg_builder_do` (only needed if you use that macro; the header includes it unconditionally)

Inside the mulle-core amalgamation, all dependencies are provided by mulle-core itself.
