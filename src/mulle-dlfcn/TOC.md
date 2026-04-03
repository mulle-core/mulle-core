# mulle-dlfcn Library Documentation for AI
<!-- Keywords: dynamic-loading, plugins -->

## 1. Introduction & Purpose

`mulle-dlfcn` is a C library that provides a portable, cross-platform interface for dynamically loading shared libraries at runtime. It abstracts the platform-specific APIs for library loading (like `dlfcn.h` on POSIX and the Windows API) into a single, consistent set of functions.

Its primary purpose is to allow an application to load libraries, look up symbols (functions or variables) within them by name, and then close the libraries, all without needing to link against them at compile time. This is crucial for implementing features like plugin systems, modular architectures, and just-in-time (JIT) compilers. For Windows, it seamlessly integrates the `dlfcn-win32` compatibility library to provide the standard POSIX `dlfcn` API.

## 2. Key Concepts & Design Philosophy

The library's design is that of a **conforming wrapper**. It provides functions that directly map to the standard POSIX `dlfcn` API.

- **Abstraction:** On POSIX-compliant systems (Linux, macOS), `mulle-dlfcn` is a very thin wrapper over the system's native `<dlfcn.h>`.
- **Compatibility:** On Windows, it pulls in the `dlfcn-win32` dependency, which implements the `dlopen`, `dlsym`, `dlclose`, and `dlerror` functions using the underlying Windows API (`LoadLibrary`, `GetProcAddress`, etc.). This allows code written for POSIX to compile and run on Windows with little to no modification.
- **Error Handling:** The library adopts the standard `dlfcn` error handling mechanism, where functions like `mulle_dlopen` and `mulle_dlsym` return `NULL` on error, and a detailed error message can be retrieved via `mulle_dlerror`.

## 3. Core API & Data Structures

The entire API is exposed through the `mulle-dlfcn.h` header. It does not define any public data structures, instead using an opaque `void *` handle to represent a loaded library.

### 3.1. `mulle-dlfcn.h`

#### Library Loading Functions
- `mulle_dlopen(filename, flags)`: Attempts to load the shared library specified by `filename`. The `flags` argument controls how symbols are resolved (e.g., `RTLD_LAZY`, `RTLD_NOW`). Returns an opaque handle on success, or `NULL` on failure. If `filename` is `NULL`, it returns a handle to the main executable itself.
- `mulle_dlclose(handle)`: Closes a library that was opened with `mulle_dlopen`.

#### Symbol Resolution
- `mulle_dlsym(handle, symbol_name)`: Searches for a symbol with the given `symbol_name` within the library specified by `handle`. Returns a pointer to the symbol (which can be a function or a variable) on success, or `NULL` if the symbol is not found.
- `MULLE_RTLD_DEFAULT`: A macro representing a special handle to be used with `mulle_dlsym` to search for a symbol in the main executable and all loaded libraries in the global scope.

#### Error Handling
- `mulle_dlerror(void)`: Returns a pointer to a null-terminated string describing the last error that occurred during a call to `mulle_dlopen`, `mulle_dlsym`, or `mulle_dlclose`. If no error has occurred since the last call to `mulle_dlerror`, it returns `NULL`.

## 4. Performance Characteristics

- **Overhead:** The wrapper itself adds a negligible amount of overhead. The performance of loading libraries and looking up symbols is almost entirely dictated by the underlying operating system's dynamic loader.
- **Memory Usage:** The memory usage is determined by the size of the libraries being loaded into the process's address space. The handles themselves are just pointers.
- **Thread-Safety:** The underlying `dlfcn` implementations on modern POSIX systems and the `dlfcn-win32` library are generally thread-safe.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:** Always check the return value of `mulle_dlopen` and `mulle_dlsym` for `NULL`. If `NULL` is returned, immediately call `mulle_dlerror` to get a human-readable error message for logging or debugging.
- **Error Handling:** The pointer returned by `mulle_dlerror` is only valid until the next call to a `mulle-dlfcn` function. You must copy the string if you need to store it.
- **Function Pointers:** When using `mulle_dlsym` to load a function, the returned `void *` must be cast to the correct function pointer type before it can be called. Be aware that casting a `void *` to a function pointer is technically undefined behavior in ISO C, but it is a common and required practice for this API on POSIX systems.
- **Portability:** Use this library instead of directly calling `LoadLibrary` on Windows or `<dlfcn.h>` functions on POSIX to ensure your code is portable.

## 6. Integration Examples

### Example 1: Loading a Library and Calling a Function

This example demonstrates the fundamental workflow of loading a shared library (in this case, the standard C math library), looking up the `cos` function, calling it, and then closing the library.

```c
#include <mulle-dlfcn/mulle-dlfcn.h>
#include <stdio.h>

// Define the function pointer type for cos()
typedef double (*cos_function_ptr)(double);

int main(int argc, char *argv[])
{
    void *handle;
    cos_function_ptr cos_func;
    char *error;

    // On Linux, this would be "libm.so.6"; on macOS, "libm.dylib".
    // Passing NULL on Windows would require the library to be in the search path.
    // For this example, we assume a platform-specific name.
#if defined(_WIN32)
    const char *lib_name = "msvcrt.dll"; // A library that contains cos
#else
    const char *lib_name = "libm.so.6"; // A common name for the math library on Linux
#endif

    handle = mulle_dlopen(lib_name, RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "Failed to load library: %s\n", mulle_dlerror());
        return 1;
    }

    // Clear any existing error
    mulle_dlerror();

    // Look up the 'cos' symbol
    // Note the required cast from void* to the function pointer type
    cos_func = (cos_function_ptr)mulle_dlsym(handle, "cos");

    error = mulle_dlerror();
    if (error != NULL)
    {
        fprintf(stderr, "Failed to find symbol 'cos': %s\n", error);
        mulle_dlclose(handle);
        return 1;
    }

    // Call the function
    double result = cos_func(2.0);
    printf("The cosine of 2.0 is %f\n", result);

    // Close the library
    mulle_dlclose(handle);

    return 0;
}
```

## 7. Dependencies

- `mulle-c11`
- `dlfcn-win32` (on Windows platforms)
- `dl` (on POSIX platforms)
- `psapi` (on Windows platforms)
