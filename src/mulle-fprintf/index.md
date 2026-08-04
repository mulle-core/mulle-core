# mulle-fprintf Library Documentation for AI
<!-- Keywords: printf, stdio -->

## 1. Introduction & Purpose

mulle-fprintf bridges mulle-sprintf with stdio.h, providing printf-family functions that write to FILE streams while supporting the full mulle-sprintf feature set including custom format specifiers, varargs and mulle-vararg style arguments, UTF-32/UTF-16 output, and Objective-C extensions. It serves as a drop-in replacement for standard fprintf/printf with extended capabilities.

## 2. Key Concepts & Design Philosophy

- **Sprintf Wrapper**: Builds on mulle-sprintf; all format specifiers and extensions work identically
- **Two Argument Styles**: Supports both traditional C varargs (va_list) and optimized mulle-vararg style
- **Stream Abstraction**: Works with any FILE * output stream (stdout, stderr, files, pipes)
- **Format Extensibility**: Like mulle-sprintf, custom format conversions can be registered
- **Seamless Integration**: Direct drop-in for printf/fprintf with full backward compatibility

## 3. Core API & Data Structures

### mulle-fprintf.h - Core Functions

#### Standard Printf-Like Functions

- `mulle_printf(format, ...)` → `int`: Printf to stdout; equivalent to fprintf(stdout, ...)
- `mulle_vprintf(format, args)` → `int`: Printf to stdout using va_list
- `mulle_mvprintf(format, arguments)` → `int`: Printf to stdout using mulle_vararg_list
- `mulle_fprintf(fp, format, ...)` → `int`: Printf to FILE stream with variadic arguments
- `mulle_vfprintf(fp, format, args)` → `int`: Printf to FILE stream with va_list
- `mulle_mvfprintf(fp, format, arguments)` → `int`: Printf to FILE stream with mulle_vararg_list

#### Version Functions

- `mulle_fprintf_get_version()` → `uint32_t`: Returns version number (major.minor.patch)
- `mulle_fprintf_get_version_major()` → `unsigned int`: Extracts major version
- `mulle_fprintf_get_version_minor()` → `unsigned int`: Extracts minor version
- `mulle_fprintf_get_version_patch()` → `unsigned int`: Extracts patch version

### mulle-buffer-stdio.h - Buffer to Stream

#### Buffer I/O Integration

- `mulle_buffer_fprint(fp, buffer)` → `int`: Writes mulle_buffer content to FILE stream
- `mulle_buffer_print(buffer)` → `int`: Writes mulle_buffer to stdout (convenience)

### mulle_buffer Functions (from mulle-buffer dependency)

#### Buffer Lifecycle

- `mulle_buffer_make_zero()` → `struct mulle_buffer`: Creates an empty buffer
- `mulle_buffer_done(buffer)` → `void`: Releases buffer resources

#### Buffer Formatting

- `mulle_buffer_sprintf(buffer, format, ...)` → `void`: Appends formatted text to buffer (from mulle-buffer)

## 4. Performance Characteristics

- **O(n) for output**: Linear in formatted output length
- **Single pass**: Formats once then writes to stream (no intermediate buffering overhead vs. snprintf variants)
- **No internal buffering**: Delegates buffering to FILE stream (stdio handles)
- **Flush behavior**: Depends on stream (line-buffered for stdout, fully buffered for files)
- **Thread-Safety**: FILE stream thread-safety depends on platform; mulle-fprintf adds no additional synchronization
- **Memory**: Minimal stack usage for formatting; no dynamic allocation

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Use mulle_printf for stdout**: Simpler than mulle_fprintf(stdout, ...)
- **Stream Flushing**: Use fflush(fp) after critical output if stream is not line-buffered
- **Error Checking**: Return value is character count; -1 may indicate stream error
- **Varargs Choice**: Use mulle_mvfprintf for new code (more efficient); use mulle_vfprintf for C compatibility
- **Format Specifiers**: All mulle-sprintf specifiers work here (see mulle-sprintf documentation)

### Common Pitfalls

- **FILE * Validity**: Ensure FILE * is open and valid before calling
- **Format String**: No runtime validation; invalid format strings produce undefined behavior
- **Stream State**: Errors in previous operations can affect subsequent calls; check ferror(fp)
- **Buffering Mode**: stdout is line-buffered by default; may see delayed output if not flushed
- **Custom Conversions**: No way to pass custom conversion table to mulle_fprintf (use mulle-buffer + mulle_sprintf instead)

### Idiomatic Usage

```c
// Pattern 1: Simple logging
mulle_printf("Debug: value=%d\n", x);
mulle_fprintf(stderr, "Error: %s\n", msg);

// Pattern 2: Build in buffer, write to stream
struct mulle_buffer buf = mulle_buffer_make_zero();
mulle_buffer_sprintf(&buf, "formatted: %d", x);
mulle_buffer_fprint(stdout, &buf);

// Pattern 3: Variadic forwarding
void log_info(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    mulle_vfprintf(stdout, fmt, va);
    va_end(va);
}
```

## 6. Integration Examples

### Example 1: Basic Printf Output

```c
#include <mulle-fprintf/mulle-fprintf.h>

int main() {
    mulle_printf("Hello, %s!\n", "world");
    mulle_printf("Number: %d, Hex: %#x\n", 255, 255);
    return 0;
}
```

### Example 2: Writing to File

```c
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>

int main() {
    FILE *fp = fopen("output.txt", "w");
    if (!fp) return 1;
    
    mulle_fprintf(fp, "Line 1: %s\n", "data");
    mulle_fprintf(fp, "Line 2: value=%d\n", 42);
    
    fflush(fp);
    fclose(fp);
    return 0;
}
```

### Example 3: Error Reporting to stderr

```c
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>

void report_error(const char *function, int error_code) {
    mulle_fprintf(stderr, "ERROR in %s: code %d\n", function, error_code);
    fflush(stderr);
}

int main() {
    report_error("process_data", 42);
    return 0;
}
```

### Example 4: Using Variadic Forwarding

```c
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdarg.h>
#include <stdio.h>

void log_with_prefix(const char *prefix, const char *format, ...) {
    va_list va;
    
    mulle_fprintf(stdout, "[%s] ", prefix);
    
    va_start(va, format);
    mulle_vfprintf(stdout, format, va);
    va_end(va);
    
    mulle_fprintf(stdout, "\n");
}

int main() {
    log_with_prefix("INFO", "Processing %d items", 100);
    log_with_prefix("WARN", "Value out of range: %d", 9999);
    return 0;
}
```

### Example 5: Buffer to Stream

```c
#include <mulle-fprintf/mulle-fprintf.h>
#include <mulle-buffer/mulle-buffer.h>

int main() {
    struct mulle_buffer buf = mulle_buffer_make_zero();
    
    mulle_buffer_sprintf(&buf, "Header\n");
    for (int i = 0; i < 3; i++) {
        mulle_buffer_sprintf(&buf, "  Item %d\n", i + 1);
    }
    
    // Write entire buffer to stdout at once
    mulle_buffer_fprint(stdout, &buf);
    
    mulle_buffer_done(&buf);
    return 0;
}
```

### Example 6: Hex Dump with Formatting

```c
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>
#include <stdarg.h>

void hex_dump(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (i % 16 == 0)
            mulle_fprintf(stdout, "%04zx: ", i);
        
        mulle_fprintf(stdout, "%02x ", data[i]);
        
        if (i % 16 == 15 || i == len - 1)
            mulle_fprintf(stdout, "\n");
    }
}

int main() {
    unsigned char data[] = { 0x48, 0x65, 0x6c, 0x6c, 0x6f };
    hex_dump(data, sizeof(data));
    return 0;
}
```

## 7. Dependencies

- mulle-c11
- mulle-sprintf (for all format specifier support)
- mulle-buffer (for buffer I/O functions)
- mulle-vararg (for mulle_vararg support)
