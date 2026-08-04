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

- **Manual or Automatic:** Can be used with compiler support (automatic packing) or manually via builder API.

## 3. Core API & Data Structures

### 3.1 `mulle-vararg.h` - Core Vararg Access

#### Types

**`mulle_vararg_list`**

- **Purpose:** Opaque iterator for accessing variable arguments.
- **Internal:** Holds pointer to current position in argument buffer.

#### Initialization Macros

**`mulle_vararg_start(args, ap)`**

- **Purpose:** Initialize argument list from first variadic argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` to initialize.
  - `ap`: First variadic argument (name, not address).
- **Usage:** Call at start of variadic function with last named parameter.
- **Example:** `mulle_vararg_start(args, format)`

**`mulle_vararg_start_fp(args, ap)`**

- **Purpose:** Initialize argument list when first argument is floating-point.
- **Handles:** Double alignment adjustment for floating-point first arguments.

#### Integer Argument Access

**`mulle_vararg_next_integer(args, type)`**

- **Purpose:** Read and advance to next integer-like argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` iterator.
  - `type`: C integer type to extract (int, char, long, etc.).
- **Returns:** Value converted to specified type.
- **Behavior:** Handles integer promotion (small types promoted to int).

#### Floating-Point Argument Access

**`mulle_vararg_next_double(args)`**

- **Purpose:** Read and advance to next floating-point argument.
- **Returns:** Double precision float (floats promoted to double).

**`mulle_vararg_next_float(args)`**

- **Purpose:** Read and advance to next single-precision float.
- **Returns:** Float value.

#### Pointer Argument Access

**`mulle_vararg_next_pointer(args, type)`**

- **Purpose:** Read and advance to next pointer argument.
- **Parameters:**
  - `args`: `mulle_vararg_list` iterator.
  - `type`: Pointer type (e.g., `void *`, `char *`).
- **Returns:** Pointer value.

#### Raw Memory Access

**`_mulle_vararg_int_aligned_pointer(args, size, align)`**

- **Purpose:** Low-level: Get aligned pointer to next argument and advance.
- **Parameters:**
  - `args`: Pointer to `mulle_vararg_list`.
  - `size`: Argument size in bytes.
  - `align`: Required alignment.
- **Returns:** Pointer to argument data.
- **Note:** Handles integer promotion (< sizeof(int) → int).

### 3.2 `mulle-vararg-builder.h` - Manual Argument Construction

**`struct mulle_vararg_builder`**

- **Purpose:** Build argument buffer manually without compiler support.
- **Usage:** When compiler does not support mulle-vararg natively.

#### Builder Functions

**`mulle_vararg_builder_init(buffer, size)`**

- **Purpose:** Initialize builder with output buffer.
- **Parameters:**
  - `buffer`: Destination for packed arguments.
  - `size`: Buffer size in bytes.
- **Returns:** Initialized builder.

**`mulle_vararg_builder_add_integer(builder, type, value)`**

- **Purpose:** Add integer argument to buffer.
- **Behavior:** Respects promotion rules and alignment.

**`mulle_vararg_builder_add_double(builder, value)`**

- **Purpose:** Add double-precision float to buffer.

**`mulle_vararg_builder_add_pointer(builder, value)`**

- **Purpose:** Add pointer argument to buffer.

**`mulle_vararg_builder_get_data(builder)`**

- **Purpose:** Get pointer to packed argument buffer.
- **Returns:** Buffer suitable for `mulle_vararg_list_make()`.

### 3.3 `mulle-align.h` - Alignment Utilities

**`mulle_pointer_align(p, align)`**

- **Purpose:** Align pointer to specified alignment boundary.
- **Parameters:**
  - `p`: Pointer to align.
  - `align`: Alignment (typically power of 2).
- **Returns:** Aligned pointer (may be ≥ original).

## 4. Argument Layout Examples

### Example 1: Simple Integer Arguments

```
printf("x=%d", 42)

Argument buffer:
[0x00] 00000000   (unused - printf first arg is format)
[0x04] 0000002A   (int 42, promoted from implicit cast)
```

### Example 2: Mixed Types with Promotion

```
printf("%d %f", (char)'x', (float)0.2)

Argument buffer (32-bit):
[0x00] 00000078                 (int, char 'x' promoted)
[0x04] 00000000 
[0x08] 3fc99999 3fc99999       (double, float promoted)
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

1. **Use Compiler Integration:** If compiler supports mulle-vararg, use automatic packing.

2. **Order Arguments Correctly:** Always initialize with `mulle_vararg_start()` before accessing arguments.

3. **Type Safety:** Know expected argument types; mismatched reads yield garbage.

4. **Buffer Size:** Builder users must provide sufficiently large buffer (conservative estimate: 128 bytes typical).

5. **Alignment Respect:** Trust alignment macros; manual pointer arithmetic risks misalignment.

### Common Pitfalls:

1. **Accessing Arguments Out-of-Order:** Sequential access required; no random access.

2. **Type Mismatch:** Reading with wrong macro (e.g., `next_integer` instead of `next_double`) yields corrupted values.

3. **Buffer Overflow (Builder):** Insufficient buffer causes corruption; always validate.

4. **Assuming Stack Layout:** Not compatible with standard stdarg; don't mix APIs.

5. **Forgetting Promotion Rules:** Small integers are promoted to int; account for this when reading.

## 7. Integration Examples

### Example 1: Simple Printf-like Function

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>

void my_printf(const char *format, ...) {
    mulle_vararg_list args;
    mulle_vararg_start(args, format);
    
    for (const char *p = format; *p; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd': {
                    int val = mulle_vararg_next_integer(args, int);
                    printf("%d", val);
                    break;
                }
                case 'f': {
                    double val = mulle_vararg_next_double(args);
                    printf("%f", val);
                    break;
                }
                case 's': {
                    const char *s = mulle_vararg_next_pointer(args, const char *);
                    printf("%s", s);
                    break;
                }
                case '%':
                    printf("%%");
                    break;
            }
        } else {
            printf("%c", *p);
        }
    }
}

int main() {
    my_printf("Number: %d, Float: %f, String: %s\n", 42, 3.14, "hello");
    return 0;
}
```

### Example 2: Manual Argument Construction with Builder

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>
#include <string.h>

void process_args(const char *format, mulle_vararg_list args) {
    // Process args built by builder
    int i = 0;
    while (format[i]) {
        if (format[i] == 'd') {
            int val = mulle_vararg_next_integer(args, int);
            printf("Integer: %d\n", val);
        } else if (format[i] == 'f') {
            double val = mulle_vararg_next_double(args);
            printf("Float: %f\n", val);
        }
        i++;
    }
}

int main() {
    char buffer[256];
    struct mulle_vararg_builder builder;
    
    builder = mulle_vararg_builder_init(buffer, sizeof(buffer));
    
    // Build arguments: int, double, int
    mulle_vararg_builder_add_integer(&builder, int, 42);
    mulle_vararg_builder_add_double(&builder, 3.14);
    mulle_vararg_builder_add_integer(&builder, int, 99);
    
    mulle_vararg_list args = mulle_vararg_list_make(mulle_vararg_builder_get_data(&builder));
    
    process_args("dfd", args);
    
    return 0;
}
```

### Example 3: Mixed Type Processing

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>

void log_mixed(const char *prefix, ...) {
    mulle_vararg_list args;
    mulle_vararg_start(args, prefix);
    
    printf("[%s] ", prefix);
    
    // Assume format: int, double, char*
    int count = mulle_vararg_next_integer(args, int);
    double average = mulle_vararg_next_double(args);
    const char *label = mulle_vararg_next_pointer(args, const char *);
    
    printf("Count: %d, Average: %.2f, Label: %s\n", count, average, label);
}

int main() {
    log_mixed("DEBUG", 10, 4.5, "measurements");
    log_mixed("INFO", 25, 7.3, "results");
    
    return 0;
}
```

### Example 4: Type-Safe Vararg Wrapper

```c
#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>
#include <stdint.h>

typedef struct {
    int type;  // 0=int, 1=double, 2=ptr
    union {
        int i;
        double d;
        void *p;
    } value;
} Argument;

#define MAX_ARGS 16

void call_typed(const char *name, Argument *args, int count) {
    printf("Function: %s\n", name);
    
    for (int i = 0; i < count; i++) {
        printf("  Arg %d: ", i);
        switch (args[i].type) {
            case 0:
                printf("int=%d\n", args[i].value.i);
                break;
            case 1:
                printf("double=%.2f\n", args[i].value.d);
                break;
            case 2:
                printf("ptr=%p\n", args[i].value.p);
                break;
        }
    }
}

int main() {
    Argument args[] = {
        {0, {.i = 42}},
        {1, {.d = 3.14}},
        {2, {.p = (void *)0x12345678}}
    };
    
    call_typed("example", args, 3);
    
    return 0;
}
```

## 7. Dependencies

Direct mulle-sde dependencies:
- `mulle-c11`: C11 compatibility macros and alignment utilities
