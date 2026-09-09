# mulle-fprintf Library Documentation for AI
<!-- Keywords: printf, stdio, buffer, stream, fmemopen, vtable, varargs -->

## 1. Introduction & Purpose

mulle-fprintf marries `mulle-sprintf` to `stdio.h`. It provides printf-family
functions (`mulle_printf`, `mulle_fprintf`, ...) that write to `FILE *` streams
while supporting the full mulle-sprintf feature set: custom format
specifiers, traditional C varargs (`va_list`) and optimized `mulle_vararg_list`
arguments, and UTF-32/UTF-16/ObjC extensions. It is a drop-in replacement for
standard `printf`/`fprintf` with extended capabilities.

In addition it provides a cross-platform, stdio-compatible interface layered on
`mulle_buffer`, including a portable `fmemopen` replacement that works on
macOS, Linux and Windows. Two function-pointer vtables
(`mulle_stdio_functions`, `mulle_buffer_functions`) let you write I/O code once
and run it against either a `FILE *` or a `mulle_buffer *`.

mulle-fprintf is a component of the `mulle-core` umbrella library. Its direct
dependency is `mulle-sprintf`.

## 2. Key Concepts & Design Philosophy

- **Sprintf wrapper**: Builds on `mulle-sprintf`; all format specifiers and extensions work identically (via `mulle_buffer_vsprintf` / `mulle_buffer_mvsprintf`).
- **Three argument styles**: `va_list` (variadic `...`), and `mulle_vararg_list` — pick per use case.
- **Flushable buffer output**: Each printf call formats into a stack-allocated 1024-byte `mulle_flushablebuffer`, which flushes to the `FILE *` via `fwrite` (multiple batched flushes when output exceeds 1024 bytes).
- **Stdio mimicry**: The `mulle_buffer_*` I/O functions deliberately mirror `fread`/`fwrite`/`fseek`/etc. so they can be passed as callback pointers in code that accepts libc function pointers.
- **Polymorphic I/O via vtables**: `struct mulle_buffer_stdio_functions` is a function table with `void *handle` endpoints; two prepared instances dispatch to real stdio or to mulle-buffer.
- **Cross-platform fmemopen**: `mulle_buffer_fmemopen` implements libc `fmemopen` semantics on platforms that lack it (Windows) and provides consistent behavior where libc is "flakey" (darwin/linux seeking/writing edge cases).

## 3. Core API & Data Structures

### 3.1. `mulle-fprintf.h` - Core printf functions

All `const char *format` parameters are plain C format strings interpreted by mulle-sprintf. `va_list` variants take `stdarg.h` cursors; `mulle_vararg_list` variants take mulle-vararg cursors.

#### Printf to FILE / stdout

- `int mulle_printf( const char *format, ...);`
  Print to `stdout`. Equivalent to `mulle_fprintf( stdout, ...)`.
- `int mulle_vprintf( const char *format, va_list args);`
  `va_list` variant of `mulle_printf` (static inline forwarding to `mulle_vfprintf`).
- `int mulle_mvprintf( const char *format, mulle_vararg_list arguments);`
  `mulle_vararg_list` variant of `mulle_printf` (static inline forwarding to `mulle_mvfprintf`).
- `int mulle_fprintf( FILE *fp, const char *format, ...);`
  Print to a `FILE *` stream with variadic arguments.
- `int mulle_vfprintf( FILE *fp, const char *format, va_list args);`
  Print to a `FILE *` stream with a `va_list`.
- `int mulle_mvfprintf( FILE *fp, const char *format, mulle_vararg_list arguments);`
  Print to a `FILE *` stream with a `mulle_vararg_list`.
  Returns -1 with `errno = EINVAL` if `fp` or `format` is NULL.

#### Uniform stdio wrappers (static inline)

Thin, uniformly-named wrappers around libc stdio so call sites stay consistent:

- `static inline int mulle_fputc( int c, FILE *fp);` → `fputc( c, fp)`
- `static inline int mulle_putchar( int c);` → `putchar( c)`
- `static inline int mulle_fputs( const char *s, FILE *fp);` → `fputs( s, fp)`
- `static inline int mulle_puts( const char *s);` → `puts( s)`
- `static inline int mulle_fflush( FILE *fp);` → `fflush( fp)`

#### Version functions

- `#define MULLE__FPRINTF_VERSION ((0UL << 20) | (4 << 8) | 0)` — version 0.4.0.
- `uint32_t mulle_fprintf_get_version( void);` Returns the packed version number.
- `static inline unsigned int mulle_fprintf_get_version_major( void);` Extracts major version.
- `static inline unsigned int mulle_fprintf_get_version_minor( void);` Extracts minor version.
- `static inline unsigned int mulle_fprintf_get_version_patch( void);` Extracts patch version.

### 3.2. `mulle-buffer-stdio.h` - Stdio-compatible mulle_buffer I/O

All functions take `void *buffer` as the stream handle (a `struct mulle_buffer *`). They mimic libc stdio semantics: return values, EOF conventions, and `errno` behavior (e.g. EBADF on wrong direction, ENOSPC on full write, EINVAL on invalid seek/args).

#### In-memory stream lifecycle

- `void *mulle_buffer_fmemopen( void *buf, size_t size, const char *mode);`
  Cross-platform `fmemopen` replacement. `mode` is `r`, `w`, `a`, `r+`, `w+`, `a+` (optional `b` for binary). The stream is FIXED at `size` bytes and never grows. `buf != NULL`: caller provides at least `size` bytes and retains ownership. `buf == NULL`: mulle allocates `size` bytes, ownership transfers to the buffer and is freed on close. Returns a heap-allocated `struct mulle_buffer *`; NULL with `errno = EINVAL` on bad mode. For a GROWABLE in-memory stream use `mulle_buffer_create()` / `mulle_buffer_init()` instead.
- `int mulle_buffer_fclose( void *buffer);`
  Flushes and destroys the buffer struct (`mulle_buffer_destroy`). Only for heap-allocated handles (from `mulle_buffer_fmemopen` or `mulle_buffer_create`); do NOT pass a stack/embedded `mulle_buffer` — that would `free()` a stack address.

#### Character and string I/O

- `int mulle_buffer_fgetc( void *buffer);`
  Read next byte; EOF (with `errno = 0`) on end or if buffer is NULL; `errno = EBADF` if write-only.
- `int mulle_buffer_fputc( int c, void *buffer);`
  Append a byte; EOF on readonly/NULL/full (inflexible) buffers.
- `int mulle_buffer_fputs( const char *s, void *buffer);`
  Append a C string; EOF on readonly/NULL; else bytes appended (or 0).

#### Block I/O

- `size_t mulle_buffer_fread( void *dst, size_t size, size_t nmemb, void *buffer);`
  `fread`-compatible read; returns element count read (never more than available). NULL buffer returns 0.
- `size_t mulle_buffer_fwrite( void *src, size_t size, size_t nmemb, void *buffer);`
  `fwrite`-compatible write; returns `nmemb` on success. NULL buffer returns EOF; readonly → EOF with `errno = EBADF`; full inflexible buffer → 0 with `errno = ENOSPC`.

#### Positioning

- `int mulle_buffer_fseek( void *buffer, long seek, int mode);`
  `fseek`-compatible seek; `mode` is `SEEK_SET`/`SEEK_CUR`/`SEEK_END`. SEEK_END on a writeonly buffer fails with `errno = ENOSPC`. Returns 0 on success, -1 (`errno = EINVAL`) on failure, -1 (`errno = EBADF`) if buffer is NULL.
- `long mulle_buffer_ftell( void *buffer);`
  Current position as long; 0 if buffer is NULL.
- `off_t mulle_buffer_stdio_lseek( void *buffer, off_t seek, int mode);`
  `off_t`-based seek adapter for the vtable `lseek` slot (renamed from `mulle_buffer_lseek` to avoid clashing with mulle-buffer's type-safe inline). Returns new position, or -1 with `errno = EINVAL`/`EBADF`. Offsets that do not fit in `long` fail with EINVAL.

#### Flush

- `int mulle_buffer_fflush( void *buffer);`
  In text mode zero-terminates the last byte (no truncate); if the buffer is flushable, flushes it. Returns 0, or EOF with `errno = EBADF` if buffer is NULL.

#### `struct mulle_buffer_stdio_functions` and prepared vtables

```c
struct mulle_buffer_stdio_functions
{
   void    *(*fmemopen)( void *buf, size_t size, const char *mode);
   int      (*fclose)( void *buffer);
   int      (*fgetc)( void *buffer);
   size_t   (*fread)( void *dst, size_t size, size_t nmemb, void *buffer);
   int      (*fputc)( int c, void *buffer);
   int      (*fputs)( const char *s, void *buffer);
   int      (*fseek)( void *buffer, long seek, int mode);
   long     (*ftell)( void *buffer);
   size_t   (*fwrite)( void *src, size_t size, size_t nmemb, void *buffer);
   int      (*fflush)( void *buffer);

   off_t    (*lseek)( void *buffer, off_t seek, int mode);
};
```

- `struct mulle_buffer_stdio_functions mulle_stdio_functions;`
  Prepared table dispatching to real `FILE *` stdio. Its `handle` argument is a `FILE *`. On Windows the `fmemopen` slot is a stub returning NULL with EINVAL.
- `struct mulle_buffer_stdio_functions mulle_buffer_functions;`
  Prepared table dispatching to the `mulle_buffer_*` functions above. Its `handle` argument is a `struct mulle_buffer *`.

Use them to write one function parameterized by `(struct mulle_buffer_stdio_functions *, void *fp)` and call it with `(&mulle_stdio_functions, fp)` or `(&mulle_buffer_functions, &buffer)`. Passing a NULL function table defaults to `mulle_buffer_functions`.

#### Reading files into a mulle_buffer

- `int mulle_buffer_init_with_filepath( struct mulle_buffer *buffer, char *filepath, int mode, struct mulle_allocator *allocator);`
  Initializes `buffer` with a file's contents using FILE I/O (`mode`: `MULLE_BUFFER_IS_BINARY` → `"rb"`, `MULLE_BUFFER_IS_TEXT` → `"r"`). Never writes. Returns `errno` (0 on success, EINVAL for NULL filepath, EISDIR on Windows for directories, or the fopen/read error).
- `size_t mulle_buffer_fread_FILE( struct mulle_buffer *buffer, size_t size, size_t nmem, FILE *fp);`
  Reads `size*nmem` bytes (or less) from `FILE *fp` into `buffer`. Returns bytes read; 0 with `errno` set on error (NULL buffer/fp → EINVAL; zero size/nmem → EFBIG).
- `size_t mulle_buffer_fread_FILE_all( struct mulle_buffer *buffer, FILE *fp);`
  Reads the remaining bytes of `FILE *fp` into `buffer`. Returns bytes read (0 for an empty file) with `errno == 0` on success; 0 with `errno` set on failure. Non-seekable streams (pipes, sockets) are not supported and fail with errno (e.g. ESPIPE); a `LONG_MAX` size (opening a directory) yields EISDIR.

#### Convenience macros

- `mulle_buffer_do_filepath( name, filepath, mode)`
  RAII-style loop: declares a `struct mulle_buffer`, initializes it from `filepath` via `mulle_buffer_init_with_filepath`, runs the loop body with `struct mulle_buffer *name` in scope, and calls `mulle_buffer_done` when the loop exits. See README usage below.
- `mulle_flushablebuffer_do_FILE( name, fp)`
  RAII-style loop using a stack `mulle_flushablebuffer` with a 128-byte internal buffer and an internal `fwrite` flusher targeting `FILE *fp`; the body's `struct mulle_buffer *name` casts from the flushable buffer. The final flush result is intentionally discarded — initialize the flushablebuffer yourself if you need to detect a failed flush.
- `static inline size_t _mulle_flushablebuffer_fwrite( void *buf, size_t one, size_t len, void *userinfo);`
  Helper writing to `(FILE *) userinfo` via `fwrite`; used as the flusher for the macro above and by `mulle_vfprintf`.

## 4. Performance Characteristics

- **Printf path**: `mulle_vfprintf`/`mulle_mvfprintf` format into a stack-allocated 1024-byte flushable buffer (no heap allocation), then flush via `fwrite`. Output larger than 1024 bytes causes one or more additional batched flushes. O(n) in output size with minimal constant overhead.
- **Buffer I/O**: appends/reads are O(1) amortized (mulle-buffer doubling); seeks are O(1). `mulle_buffer_fread_FILE_all` does a `fseek` to end to size the allocation (one extra stream traversal) before reading.
- **Memory**: printf path uses only stack storage; no dynamic allocation. In-memory streams allocate once (`fmemopen` with `buf == NULL`).
- **Vtables**: indirect calls add one function-pointer indirection per operation.
- **Thread-safety**: Not thread-safe. No locking is added; concurrent use of a shared `FILE *` or `mulle_buffer *` must be externally synchronized. Each printf call is independent (stack buffer per call), so concurrent calls on *distinct* streams are fine.

## 5. AI Usage Recommendations & Patterns

### Best Practices

- Prefer `mulle_printf` for stdout; use `mulle_fprintf`/`mulle_mvfprintf` for other streams.
- For new code, `mulle_mvfprintf`/`mulle_mvprintf` (mulle-vararg) are more efficient than `va_list`; use `mulle_vfprintf` for plain C `va_list` compatibility.
- Use `mulle_buffer_fmemopen` for portable in-memory "files", or a plain growable `mulle_buffer` when you need the stream to grow past `size`.
- Use `mulle_stdio_functions` / `mulle_buffer_functions` vtables to write I/O code portable between `FILE *` and `mulle_buffer *`.
- Write to a `mulle_buffer` first and flush once to `FILE *` when batching output.
- Always match lifecycle: `mulle_buffer_fmemopen` → `mulle_buffer_fclose`; `mulle_buffer_fmemopen(NULL, ...)` transfers ownership; embedded/stack buffers need `mulle_buffer_init` + `mulle_buffer_done`.
- For reading a whole file, `mulle_buffer_do_filepath` auto-manages init/done.

### Common Pitfalls

- Never pass a stack/embedded `struct mulle_buffer` to `mulle_buffer_fclose` — it frees the struct, i.e. a stack address.
- `mulle_buffer_fmemopen` streams are FIXED size; writing past the end fails (ENOSPC), it does not grow. Use `mulle_buffer_create()`/`mulle_buffer_init( &b, 0, NULL)` for growable streams.
- `buf != NULL` fmemopen buffers keep caller ownership; dispose of them yourself after `mulle_buffer_fclose`.
- Check return values: several functions report errors via `errno` and EOF/0/‑1; e.g. SEEK_END on a writeonly buffer fails with `errno = ENOSPC`.
- `mulle_buffer_fread_FILE_all` needs a seekable stream; pipes/sockets are not supported.
- The former `mulle_buffer_lseek` symbol was renamed to `mulle_buffer_stdio_lseek`; don't use the old name.
- `puts`/`fputs` wrappers follow libc: `mulle_puts` adds a trailing newline, `mulle_fputs` does not.

### Idiomatic Usage

```c
// Pattern 1: variadic forwarding
int   log_info( FILE *fp, const char *format, ...)
{
   va_list   args;
   int       rval;

   va_start( args, format);
   rval = mulle_vfprintf( fp, format, args);
   va_end( args);
   return( rval);
}

// Pattern 2: build in buffer, emit once
struct mulle_buffer   buffer;

mulle_buffer_init( &buffer, 0, NULL);
mulle_buffer_sprintf( &buffer, "header %d\n", x);
mulle_buffer_sprintf( &buffer, "footer\n");
fwrite( mulle_buffer_get_bytes( &buffer),
        1,
        mulle_buffer_get_length( &buffer),
        stdout);
mulle_buffer_done( &buffer);
```

## 6. Integration Examples

### Example 1: Printf to stdout and stderr

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>

int   main( void)
{
   mulle_printf( "%d: %s\n", 1848, "VfL Bochum");
   mulle_fprintf( stderr, "error: %s\n", "something went wrong");
   return( 0);
}
```

### Example 2: Polymorphic I/O with vtables

The same function writes to either a `FILE *` or a `mulle_buffer *`:

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>

static void   write_greeting( struct mulle_buffer_stdio_functions *functions,
                              void *fp)
{
   (*functions->fputs)( "Hello", fp);
   (*functions->fputc)( '\n', fp);
}

int   main( void)
{
   struct mulle_buffer   buffer;

   write_greeting( &mulle_stdio_functions, stdout);

   mulle_buffer_init( &buffer, 64, NULL);
   write_greeting( &mulle_buffer_functions, &buffer);
   mulle_printf( "buffer contains: %.*s",
                 (int) mulle_buffer_get_length( &buffer),
                 mulle_buffer_get_bytes( &buffer));
   mulle_buffer_done( &buffer);

   return( 0);
}
```

### Example 3: Cross-platform fmemopen (fixed-size in-memory stream)

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>

int   main( void)
{
   char   buf[ 64] = "Hello World";
   void   *stream;

   stream = mulle_buffer_fmemopen( buf, sizeof( buf), "r+");
   if( ! stream)
      return( 1);

   mulle_buffer_fputc( 'J', stream);
   mulle_buffer_fclose( stream);

   mulle_printf( "%s\n", buf);   // "Jello World"
   return( 0);
}
```

### Example 4: Reading a file into a buffer

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>

int   main( void)
{
   mulle_buffer_do_filepath( buffer, "example.txt", MULLE_BUFFER_IS_TEXT)
   {
      fwrite( mulle_buffer_get_bytes( buffer),
              1,
              mulle_buffer_get_length( buffer),
              stdout);
   }
   return( 0);
}
```

### Example 5: Streaming large output through a flushable buffer

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>
#include <string.h>

int   main( void)
{
   char   big[ 1101];
   FILE   *fp;

   memset( big, 'a', 1099);
   big[ 1099] = '\n';
   big[ 1100] = '\0';

   fp = tmpfile();
   if( ! fp)
      return( 1);

   // 1100 bytes > the 1024-byte stack buffer -> multiple batched flushes
   mulle_fprintf( fp, "%s", big);
   fflush( fp);
   fclose( fp);
   return( 0);
}
```

### Example 6: mulle_vararg_list arguments

```c
#include <mulle-fprintf/mulle-fprintf.h>

#include <mulle-vararg/mulle-vararg.h>
#include <mulle-vararg/mulle-vararg-builder.h>

int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n(
                                         mulle_vararg_sizeof_integer( int)
                                       + mulle_vararg_sizeof_integer( long)
                                       + mulle_vararg_sizeof_double())];
   mulle_vararg_list             p;
   mulle_vararg_list             varargs;

   varargs = mulle_vararg_list_make( buf);
   p       = varargs;
   mulle_vararg_push_int( p, 1848);
   mulle_vararg_push_long( p, 48L);
   mulle_vararg_push_double( p, 3.5);

   mulle_mvfprintf( stdout, "%d %ld %g\n", varargs);
   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` dependency (from `.mulle/etc/sourcetree/config`):

- `mulle-sprintf` (source of all format-specifier and vararg-support behavior)

Transitive dependencies pulled in via `mulle-sprintf` (relevant to linking and API availability): `mulle-buffer`, `mulle-allocator`, `mulle-vararg`, `mulle-c11`.

mulle-fprintf is itself a component of the `mulle-core` umbrella; typical consumers include `<mulle-core/mulle-core.h>`, which brings in `mulle-fprintf/mulle-fprintf.h`.